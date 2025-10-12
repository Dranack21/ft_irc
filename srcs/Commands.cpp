#include "ft_irc.hpp"

// send_error_mess(client_fd, ERR_UNKNOWNCOMMAND, "Unknown command", command);)

void	Server_class::parse_and_execute_command(int client_fd, const std::string& complete_message)
{
	std::istringstream iss(complete_message);
    std::string command;
    iss >> command;
	if (command == "JOIN")
		handle_join_command(client_fd, iss);
	else if (command == "PRIVMSG")
		handle_priv_command(client_fd, iss);
	else if (command == "MODE")
		handle_mode_command(client_fd, iss);
	else if (command == "WHOIS")
		handle_whois_command(client_fd, iss);
	else if (command == "TOPIC")
		handle_topic_command(client_fd, iss);
	// else if (command == "WHO")
	// 	handle_who_command(client_fd, iss);
    else if (command == "PING")
    {
        std::string token;
        iss >> token;
        std::string response = ":" + server_name + " PONG " + server_name + " :" + token + "\r\n";
        send(client_fd, response.c_str(), response.length(), 0);
    }
	else
	{
		std::cout << "UNKNOWN COMMAND FROM CLIENT: " << client_fd << " "<< command << std::endl;
		send_error_mess(client_fd, ERR_UNKNOWNCOMMAND, "UNKNOW COMMAND");
	}
    
}

void Server_class::handle_mode_command(int client_fd, std::istringstream& iss) //is bare minimum to make it work for now
{
    std::string target;
    iss >> target;
    
    if (target.empty())
    {
        send_error_mess(client_fd, ERR_NEEDMOREPARAMS, "Not enough parameters", "MODE");
        return;
    }
    if (target[0] != '#')
    {
        std::string mode_str;
        iss >> mode_str;
        
        if (!mode_str.empty())
        {
            std::string response = ":" + this->clients[client_fd].get_nickname() + 
                                 " MODE " + target + " " + mode_str + "\r\n";
            send(client_fd, response.c_str(), response.length(), 0);
        }
        else
        {
            std::string response = ":" + server_name + " 221 " + 
                                 this->clients[client_fd].get_nickname() + 
                                 " +\r\n";
            send(client_fd, response.c_str(), response.length(), 0);
        }
		return;
    }
    // Channel mode
    std::string channel = target;
    if (!is_existing_channel(channel))
    {
        send_error_mess(client_fd, ERR_NOSUCHCHANNEL, "No such channel", channel);
        return;
    }
    if (!this->channels[channel].is_client_in_channel(client_fd))
    {
        send_error_mess(client_fd, ERR_NOTONCHANNEL, "You're not on that channel", channel);
        return;
    }
    std::string mode_string;
    iss >> mode_string;
    if (mode_string.empty())
    {
        std::string mode_response = build_channel_mode_string(channel);
        std::string response = ":" + server_name + " 324 " + 
                             this->clients[client_fd].get_nickname() + 
                             " " + channel + " " + mode_response + "\r\n";
        send(client_fd, response.c_str(), response.length(), 0);
        return;
    }
    if (!is_channel_operator(client_fd, channel))
    {
        send_error_mess(client_fd, ERR_CHANOPRIVSNEEDED, "You're not channel operator", channel);
        return;
    }
    apply_channel_modes(client_fd, channel, mode_string, iss);
}

//so handle only +o, -o, +k, -k, +t, -t for now

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
        switch (mode)
        {
            case 'o': // Operator privilege
            {
                std::string target_nick;
                if (!(iss >> target_nick))
                {
                    send_error_mess(client_fd, ERR_NEEDMOREPARAMS, "Not enough parameters for +/-o");
                    continue;
                }
                
                int target_fd = is_existing_client(target_nick);
                if (target_fd == -1)
                {
                    send_error_mess(client_fd, ERR_NOSUCHNICK, "No such nick", target_nick);
                    continue;
                }
                
                if (!this->channels[channel].is_client_in_channel(target_fd))
                {
                    send_error_mess(client_fd, ERR_USERNOTINCHANNEL, target_nick + " is not on channel", channel);
                    continue;
                }
                
                if (adding)
                {
                    if (!is_channel_operator(target_fd, channel))
                        this->channels[channel].Operators.push_back(target_fd);
                }
                else
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
                applied_modes += 'o';
                applied_params += " " + target_nick;
                break;
            }
                
            case 'k': // Channel password
            {
                if (adding)
                {
                    std::string key;
                    if (!(iss >> key))
                    {
                        send_error_mess(client_fd, ERR_NEEDMOREPARAMS, "Not enough parameters for +k");
                        continue;
                    }
                    this->channels[channel].password = key;
                    this->channels[channel].has_password = true;
                    applied_modes += 'k';
                    applied_params += " " + key;
                }
                else
                {
                    this->channels[channel].has_password = false;
                    this->channels[channel].password.clear();
                    applied_modes += 'k';
                }
                break;
            }
                
            case 't': // Topic restriction
                this->channels[channel].topic_restricted = adding;
                applied_modes += 't';
                break;
                
            default:

                send_error_mess(client_fd, ERR_UNKNOWNMODE, std::string("Unknown mode character: ") + mode);
                continue;
        }
    }
    
    // Broadcast mode change to all channel members if anything changed
    if (!applied_modes.empty() && applied_modes != "+" && applied_modes != "-")
    {
        std::string mode_msg = ":" + get_client_prefix(this->clients[client_fd]) +
                              " MODE " + channel + " " + applied_modes + applied_params + "\r\n";
        
        // Send to all channel members including the one who set it
        send_message_to_channel(client_fd, channel, mode_msg);
        send(client_fd, mode_msg.c_str(), mode_msg.length(), 0);
        
        server_history("MODE " + channel + " " + applied_modes + applied_params + " by " + 
                      this->clients[client_fd].get_nickname());
    }
}

void	Server_class::handle_priv_command(int client_fd, std::istringstream& iss)
{
	int	receiver_fd;
	std::string					message, receivers_str, temp;
	std::vector<std::string>	receivers;
	std::string					client_sender_nick;

	if (!(iss >> receivers_str))
		return(send_error_mess(client_fd, ERR_NEEDMOREPARAMS, "PRIVMSG :Need at least a recipient"));
	if (!(iss >> message))
		return (send_error_mess(client_fd, ERR_NEEDMOREPARAMS, "PRIVMSG :Need a message to be sent and at least a recipient"));
	if (message[0] != ':')
		return(send_error_mess(client_fd, ERR_UNKNOWNERROR, "PRIVMSG :Invalid synthax"));

	message = message.substr(1);
	std::cout << "DEBUG PRIVMSG: receivers_str = '" << receivers_str << "'" << std::endl;
	client_sender_nick = this->clients[client_fd].get_nickname();
	receivers = Split_by_comma(receivers_str);
	std::cout << "DEBUG PRIVMSG: receivers = [";
	for (size_t i = 0; i < receivers.size(); i++) {
		std::cout << "'" << receivers[i] << "'";
		if (i < receivers.size() - 1) std::cout << ", ";
	}
	std::cout << "]" << std::endl;
	while (iss >> temp)
		message += " " + temp;
	message += "\r\n";
	
	for(std::vector<std::string>::iterator it = receivers.begin(); it != receivers.end(); it++)
	{
		receiver_fd = (is_existing_client(*it));
		if (receiver_fd != -1)
		{
			std::string formatted = ":" + client_sender_nick + "!~" + this->clients[client_fd].get_username() + "@localhost PRIVMSG " + *it + " :" + message;
			send(receiver_fd, formatted.c_str(), formatted.size(), 0);
			std::cout << "Client fd: " << receiver_fd << std::endl;
			std::cout <<  "message: " << message << std::endl;
			server_history("Relaying message from " + client_sender_nick + " to " + *it);
		}
		else if (is_existing_channel(*it))
		{
			std::string formatted = ":" + client_sender_nick + "!~" + this->clients[client_fd].get_username() + "@localhost PRIVMSG " + *it + " :" + message;
			send_message_to_channel(client_fd, *it, formatted.c_str());
			server_history(client_sender_nick + " sent a message on channel: " + *it);
		}
		else
		{
			send_error_mess(client_fd, 401, *it + " :No such nick/channel");
		}
	}
}

void	Server_class::handle_join_command(int client_fd, std::istringstream& iss)
{
	bool						has_keys = true;
	std::vector<std::string> 	channels;
	std::vector<std::string> 	keys;
	std::string					channels_str, key, extra;
	
	if (!(iss >> channels_str))
	{
		send_error_mess(client_fd, ERR_NEEDMOREPARAMS, "Not enough parameters");
    	return;
    }
	if (!(iss >> key))
		has_keys = false;
	if (iss >> extra)
	{
		send_error_mess(client_fd, ERR_UNKNOWNCOMMAND, "Invalid synthax for JOIN command");
		return ;
	}
	channels = Split_by_comma(channels_str);
	if (has_keys == true)
		keys = Split_by_comma(key);
	for (std::vector<std::string>::iterator it = channels.begin(); it < channels.end(); it++)
	{
		if (!check_if_valid_channel_name(*it))  
			send_error_mess(client_fd, ERR_NOSUCHCHANNEL, "Invalid characters found in channel name");
		else
			Join_channel(client_fd, *it, keys);
	}
}

void	Server_class::handle_pass_command(int client_fd, std::istringstream& iss)
{
	std::string password;
	
	if(!(iss >> password))
	{
		send_error_mess(client_fd, ERR_NEEDMOREPARAMS, "Not enough parameters", "PASS");
		disconnect_client(client_fd);
		return;
	}
	if (password.empty())
	{
		send_error_mess(client_fd, ERR_NEEDMOREPARAMS, "Not enough parameters", "PASS");
		disconnect_client(client_fd);

		return;
	}
	
	if (this->clients[client_fd].is_authenticated())
	{
		send_error_mess(client_fd, ERR_ALREADYREGISTRED, "You may not reregister");
		return;
	}
	
	if (password == this->server_password)
	{
		this->clients[client_fd].set_authenticated(true);
		check_registration_complete(client_fd);
	}
	else
	{
		send_error_mess(client_fd, ERR_PASSWDMISMATCH, "Password incorrect, closing the connection");
		disconnect_client(client_fd);
	}
}

void	Server_class::handle_user_command(int client_fd, std::istringstream& iss)
{
	std::string username, hostname, servername, realname;
	iss >> username >> hostname >> servername;
	
	std::string remaining;
	std::getline(iss, remaining);
	if (!remaining.empty() && remaining[0] == ' ')
		remaining = remaining.substr(1);
	if (!remaining.empty() && remaining[0] == ':')
		realname = remaining.substr(1);
	else
		realname = remaining;
	
	if (username.empty())
	{
		send_error_mess(client_fd, ERR_NEEDMOREPARAMS, "Not enough parameters", "USER");
		return;
	}
	
	if (this->clients[client_fd].has_username())
	{
		send_error_mess(client_fd, ERR_ALREADYREGISTRED, "You may not reregister");
		return;
	}
	
	this->clients[client_fd].set_realname(realname); //forgor to set realname
	this->clients[client_fd].set_username(username);
	std::cout << "Client " << client_fd << " set username to: " << username << std::endl;

	check_registration_complete(client_fd);
}

void	Server_class::handle_nick_command(int client_fd, std::istringstream& iss)
{
	std::string	nickname;
	iss >> nickname;
	
	if (nickname.empty())
	{
		send_error_mess(client_fd, ERR_NONICKNAMEGIVEN, "No nickname given");
		return;
	}
	if (is_nickname_in_use(nickname))
	{
		send_error_mess(client_fd, ERR_NICKNAMEINUSE, "Nickname is already in use", nickname);
		return;
	}
	if (!is_valid_nickname(nickname))
	{
		send_error_mess(client_fd, ERR_ERRONEUSNICKNAME, "Erroneous nickname", nickname);
		return;
	}
	if (this->clients[client_fd].has_nickname() && this->clients[client_fd].is_fully_authenticated()) // if already has a nick so is to change it 
	{
		std::string old_prefix = get_client_prefix(this->clients[client_fd]);
		this->clients[client_fd].set_nickname(nickname);
		std::string message = ":" + old_prefix + " NICK " + nickname + "\r\n";
		send(client_fd, message.c_str(), message.length(), 0);
	}
	else
	{
		this->clients[client_fd].set_nickname(nickname);
	}
	std::cout << "Client " << client_fd << " set nickname to: " << nickname << std::endl;
	check_registration_complete(client_fd);
}

void	Server_class::handle_whois_command(int client_fd, std::istringstream &iss)
{
	std::string nick;
	std::string requester_nick;
	std::string buffer;
	std::map<int, Client>::iterator it;

	it = this->clients.begin();
	requester_nick = this->clients[client_fd].get_nickname();

	if (!(iss >> nick))
		send_error_mess(client_fd, 461, requester_nick + " WHOIS :Not enough parameters\r\n");
	else
	{
		while(it != this->clients.end())
		{
			//WHO IS FORMAT "SERVER ERROR_CODE REQUESTER_NICK NICK USER LOCALHOST : REALNAME"
			if (it->second.get_nickname() == nick)
			{	
				buffer = ":ft_irc.42.fr 311 " + requester_nick + " " + it->second.get_nickname() + " " + it->second.get_username() + " localhost * :" + it->second.get_realname() + "\r\n";
				send(client_fd, buffer.c_str(), buffer.length(), 0);
				buffer = nick + " :End of /WHOIS list\r\n";
				send_error_mess(client_fd, 318, buffer.c_str());
				break;
			}
			it++;
		}
		if (it == this->clients.end())
		{
			buffer = requester_nick + " " + nick + " :No such nick/channel\r\n";
			send_error_mess(client_fd, 401, buffer);
		}
	}
}

void	Server_class::handle_topic_command(int client_fd, std::istringstream &iss)
{
	std::string arg;
	std::string	channel_name;
	std::string topic;

	if(!(iss >> arg))
	{
		std::cout << "IDKKK" << std::endl;
	}
	else if (arg == "-delete")
	{
		if (!(iss >> channel_name))
			send_error_mess(client_fd, 461, "TOPIC :-delete must be followed up with a channel");
		else if (channel_name[0] != '#' || is_existing_channel(channel_name) || this->channels[channel_name].is_client_in_channel(client_fd))
			send_error_mess(client_fd, 403, channel_name + " :No such channel");
		else
			this->channels[channel_name].topic.clear();
	}
	else if (is_existing_channel(arg))
	{
		channel_name = arg;
		if (iss >> arg)
		{
			if (arg[0] == ':')
				topic += arg.substr(1, arg.length() -1);
			else
				topic += arg;
		}
		while (iss >> arg)
			topic += arg;
		this->channels[channel_name].topic = topic;
		std::cout << "TOPIC :" << topic << std::endl;
		std::string response = ":" + this->clients[client_fd].get_nickname() + "!" + this->clients[client_fd].get_username() + "@" + server_name + " TOPIC " + channel_name + " :" + topic + "\r\n";
		std::cout << response << std::endl;
		send(client_fd, response.c_str(), response.length(), 0);
	}
}

// void Server_class::handle_who_command(int client_fd, std::istringstream& iss)
// {
//     std::string target;
//     iss >> target;
//     std::string requester_nick = this->clients[client_fd].get_nickname();
    
//     if (target.empty())
//     {
//         send_error_mess(client_fd, RPL_ENDOFWHO, "* :End of WHO list");
//         return;
//     }
//     if (target[0] == '#')
//     {
//         if (!is_existing_channel(target))
//         {
//             send_error_mess(client_fd, ERR_NOSUCHCHANNEL, "No such channel", target);
//             return;
//         }
//         std::vector<int>::iterator it;
//         for (it = this->channels[target].Clients.begin(); 
//              it != this->channels[target].Clients.end(); ++it)
//         {
//             Client& user = this->clients[*it];
//             std::string flags = "H";
//             if (is_channel_operator(*it, target))
//                 flags += "@";
//             std::string who_reply = ":" + server_name + " 352 " + requester_nick + " " +  target + " " + user.get_username() + " localhost " + server_name + " " + user.get_nickname() + " " + flags + " :0 " + user.get_realname() + "\r\n";
//             send(client_fd, who_reply.c_str(), who_reply.length(), 0);
//         }
//         std::string end_msg = ":" + server_name + " 315 " + requester_nick + " " + target + " :End of WHO list\r\n";
//         send(client_fd, end_msg.c_str(), end_msg.length(), 0);
//     }
//     else
//     {
//         int target_fd = is_existing_client(target);
//         if (target_fd == -1)
//         {
//             send_error_mess(client_fd, ERR_NOSUCHNICK, "No such nick", target);
//             return;
//         }
//         Client& user = this->clients[target_fd];
//         std::string who_reply = ":" + server_name + " 352 " + requester_nick + " * " + user.get_username() + " localhost " + server_name + " " + user.get_nickname() + " H :0 " + user.get_realname() + "\r\n";
//         send(client_fd, who_reply.c_str(), who_reply.length(), 0);
//         std::string end_msg = ":" + server_name + " 315 " + requester_nick + " " + target + " :End of WHO list\r\n";
//         send(client_fd, end_msg.c_str(), end_msg.length(), 0);
//     }
// }

//<nick> <user> <host> * :<real_name>
//
//if command == "WHO":
//    if no argument:
//        -> optional: list all visible users
//    else if argument starts with '#':
//        -> find the channel
//        -> send 352 for each user
//        -> send 315 (End)
		//