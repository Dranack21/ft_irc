#include "ft_irc.hpp"

//so handle only +o, -o, +k, -k, +t, -t for now o l i 

void Server_class::apply_channel_modes(int client_fd, const std::string& channel, const std::string& mode_string, std::istringstream& iss) 
{
    bool adding = true;
    std::string applied_modes = "";
    std::string applied_params = "";
    
    for (size_t i = 0; i < mode_string.length(); i++)
    {
        char mode = mode_string[i];
        
        if (mode == '+')
        {
            adding = true;
            if (applied_modes.empty() || applied_modes[applied_modes.length()-1] != '+')
                applied_modes += '+';
            continue;
        }
        else if (mode == '-')
        {
            adding = false;
            if (applied_modes.empty() || applied_modes[applied_modes.length()-1] != '-')
                applied_modes += '-';
            continue;
        }
        std::string param = "";
        if (process_single_mode(client_fd, channel, mode, adding, iss, param))
        {
            applied_modes += mode;
            if (!param.empty())
                applied_params += " " + param;
        }
    }
    broadcast_mode_changes(client_fd, channel, applied_modes, applied_params);
}

bool Server_class::process_single_mode(int client_fd, const std::string& channel, char mode, bool adding, std::istringstream& iss, std::string& param)
{
    switch (mode)
    {
        case 'o':
            return handle_operator_mode(client_fd, channel, adding, iss, param);
        case 'k':
            return handle_key_mode(client_fd, channel, adding, iss, param);
        case 't':
            return handle_topic_mode(client_fd, channel, adding);
        case 'i':
            return handle_invite_mode(client_fd, channel, adding);
        case 'l':
            return handle_limit_mode(client_fd, channel, adding, iss, param);
        default:
            send_error_mess(client_fd, ERR_UNKNOWNMODE, std::string("Unknown mode character: ") + mode);
            return false;
    }
}

bool Server_class::handle_operator_mode(int client_fd, const std::string& channel, bool adding, std::istringstream& iss, std::string& param)
{
    std::string target_nick;
    if (!(iss >> target_nick))
    {
        send_error_mess(client_fd, ERR_NEEDMOREPARAMS, "Not enough parameters for +/-o");
        return false;
    }
    int target_fd = is_existing_client(target_nick);
    if (target_fd == -1)
    {
        send_error_mess(client_fd, ERR_NOSUCHNICK, "No such nick", target_nick);
        return false;
    }
    if (!this->channels[channel].is_client_in_channel(target_fd))
    {
        send_error_mess(client_fd, ERR_USERNOTINCHANNEL, target_nick + " is not on channel", channel);
        return false;
    }
    if (adding)
    {
        if (!is_channel_operator(target_fd, channel))
			this->channels[channel].Operators.push_back(target_fd);
    }
    else
    {
        remove_operator_status(target_fd, channel);
    }
    param = target_nick;
    return true;
}

bool Server_class::handle_key_mode(int client_fd, const std::string& channel, bool adding, std::istringstream& iss,std::string& param)
{
    if (adding)
    {
        std::string key;
        if (!(iss >> key))
        {
            send_error_mess(client_fd, ERR_NEEDMOREPARAMS, "Not enough parameters for +k");
            return false;
        }
        this->channels[channel].password = key;
        this->channels[channel].has_password = true;
        param = key;
    }
    else
    {
        this->channels[channel].has_password = false;
        this->channels[channel].password.clear();
    }
    return true;
}

bool Server_class::handle_invite_mode(int client_fd, const std::string& channel, 
                                      bool adding)
{
    (void)client_fd;
    this->channels[channel].invite_only = adding;
    return true;
}

bool Server_class::handle_limit_mode(int client_fd, const std::string& channel, 
                                     bool adding, std::istringstream& iss, 
                                     std::string& param)
{
    if (adding)
    {
        std::string limit_str;
        if (!(iss >> limit_str))
        {
            send_error_mess(client_fd, ERR_NEEDMOREPARAMS, 
                          "Not enough parameters for +l");
            return false;
        }
        
        // Validate that limit_str is a positive number
        char* endptr;
        long limit = strtol(limit_str.c_str(), &endptr, 10);
        
        if (*endptr != '\0' || limit <= 0 || limit > 999)
        {
            send_error_mess(client_fd, ERR_UNKNOWNERROR, 
                          "Invalid limit value (must be 1-999)");
            return false;
        }
        
        this->channels[channel].user_limit = (int)limit;
        param = limit_str;
    }
    else
    {
        // -l removes the limit
        this->channels[channel].user_limit = 0;
    }
    return true;
}





















bool Channel::is_client_invited(int client_fd)
{
    std::vector<int>::iterator it;
    for (it = invited_users.begin(); it != invited_users.end(); ++it)
    {
        if (*it == client_fd)
            return true;
    }
    return false;
}

bool Server_class::handle_topic_mode(int client_fd, const std::string& channel, bool adding)
{
    (void)client_fd;
    this->channels[channel].topic_restricted = adding;
    return true;
}

void Server_class::remove_operator_status(int target_fd, const std::string& channel)
{
    std::vector<int>::iterator it;
    for (it = this->channels[channel].Operators.begin(); 
         it != this->channels[channel].Operators.end(); ++it)
    {
        if (*it == target_fd)
        {
            this->channels[channel].Operators.erase(it);
            break;
        }
    }
}

void Server_class::broadcast_mode_changes(int client_fd, const std::string& channel, const std::string& applied_modes, const std::string& applied_params)
{
    if (applied_modes.empty() || applied_modes == "+" || applied_modes == "-")
        return;
    
    std::string mode_msg = ":" + get_client_prefix(this->clients[client_fd]) + " MODE " + channel + " " + applied_modes + applied_params + "\r\n";
    send_message_to_channel(client_fd, channel, mode_msg);
    send(client_fd, mode_msg.c_str(), mode_msg.length(), 0);
    server_history("MODE " + channel + " " + applied_modes + applied_params + " by " + this->clients[client_fd].get_nickname());
}
