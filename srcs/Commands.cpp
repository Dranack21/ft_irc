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
	else if (command == "QUIT")
		handle_quit_command(client_fd, iss);
	else if (command == "MODE")
		handle_mode_command(client_fd, iss);
	else if (command == "WHOIS")
		handle_whois_command(client_fd, iss);
	else if (command == "TOPIC")
		handle_topic_command(client_fd, iss);
	else if (command == "WHO")
		handle_who_command(client_fd, iss);
	else if (command == "INVITE")
        handle_invite_command(client_fd, iss);
	 else if (command == "KICK")
        handle_kick_command(client_fd, iss);
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
// KICK <channel> <user> [:<reason>]
void Server_class::handle_kick_command(int client_fd, std::istringstream& iss)
{
    std::string channel_name;
	std::string reason;
	std::string target_nick;
	int target_fd;
    std::string kick_msg;

    if (!(iss >> channel_name >> target_nick))
    {
        send_error_mess(client_fd, ERR_NEEDMOREPARAMS, "Not enough parameters", "KICK");
        return;
    }
    std::getline(iss, reason);
    if (!reason.empty() && reason[0] == ' ') reason = reason.substr(1);
    if (!reason.empty() && reason[0] == ':') reason = reason.substr(1);
    if (reason.empty()) reason = this->clients[client_fd].get_nickname();
    
    if (!is_existing_channel(channel_name))
    {
        send_error_mess(client_fd, ERR_NOSUCHCHANNEL, "No such channel", channel_name);
        return;
    }
    if (!this->channels[channel_name].is_client_in_channel(client_fd))
    {
        send_error_mess(client_fd, ERR_NOTONCHANNEL, "You're not on that channel", channel_name);
        return;
    }
    if (!is_channel_operator(client_fd, channel_name))
    {
        send_error_mess(client_fd, ERR_CHANOPRIVSNEEDED, "You're not channel operator", channel_name);
        return;
    }
    target_fd = is_existing_client(target_nick);
    if (target_fd == -1)
    {
        send_error_mess(client_fd, ERR_NOSUCHNICK, "No such nick", target_nick);
        return;
    }
    if (!this->channels[channel_name].is_client_in_channel(target_fd))
    {
        send_error_mess(client_fd, ERR_USERNOTINCHANNEL, target_nick + " is not on channel", channel_name);
        return;
    }
    kick_msg = ":" + get_client_prefix(this->clients[client_fd]) + " KICK " + channel_name + " " + target_nick + " :" + reason + "\r\n";
    std::vector<int>::iterator it;
    for (it = this->channels[channel_name].Clients.begin(); it != this->channels[channel_name].Clients.end(); ++it)
        send(*it, kick_msg.c_str(), kick_msg.length(), 0);

    it = std::find(this->channels[channel_name].Clients.begin(), this->channels[channel_name].Clients.end(),target_fd);
    if (it != this->channels[channel_name].Clients.end())
        this->channels[channel_name].Clients.erase(it);
    
    it = std::find(this->channels[channel_name].Operators.begin(), this->channels[channel_name].Operators.end(), target_fd);
    if (it != this->channels[channel_name].Operators.end())
        this->channels[channel_name].Operators.erase(it);
 
    if (this->channels[channel_name].Clients.empty())
        this->channels.erase(channel_name);
    transfer_operator_on_disconnect(client_fd);
    server_history(this->clients[client_fd].get_nickname() + " kicked " + target_nick + " from " + channel_name);
}

///invite <client> <channel>
void Server_class::handle_invite_command(int client_fd, std::istringstream& iss)
{
    std::string target_nick, channel_name;
	int target_fd;
    
    if (!(iss >> target_nick >> channel_name))
    {
        send_error_mess(client_fd, ERR_NEEDMOREPARAMS, "Not enough parameters", "INVITE");
        return;
    }
    if (!is_existing_channel(channel_name))
    {
        send_error_mess(client_fd, ERR_NOSUCHCHANNEL, "No such channel", channel_name);
        return;
    }
    if (!this->channels[channel_name].is_client_in_channel(client_fd))
    {
        send_error_mess(client_fd, ERR_NOTONCHANNEL, "You're not on that channel", channel_name);
        return;
    }
    if (this->channels[channel_name].invite_only && !is_channel_operator(client_fd, channel_name))
    {
        send_error_mess(client_fd, ERR_CHANOPRIVSNEEDED, "You're not channel operator", channel_name);
        return;
    }
    target_fd = is_existing_client(target_nick);
    if (target_fd == -1)
    {
        send_error_mess(client_fd, ERR_NOSUCHNICK, "No such nick", target_nick);
        return;
    }
    if (this->channels[channel_name].is_client_in_channel(target_fd))
    {
        send_error_mess(client_fd, ERR_USERONCHANNEL, target_nick + " is already on channel", channel_name);
        return;
    }
    if (!this->channels[channel_name].is_client_invited(target_fd))
        this->channels[channel_name].invited_users.push_back(target_fd);
		
    std::string invite_msg = ":" + get_client_prefix(this->clients[client_fd]) + " INVITE " + target_nick + " :" + channel_name + "\r\n";
    send(target_fd, invite_msg.c_str(), invite_msg.length(), 0);

    std::string confirm_msg = ":" + server_name + " 341 " + this->clients[client_fd].get_nickname() + " " + target_nick + " " + channel_name + "\r\n";
    send(client_fd, confirm_msg.c_str(), confirm_msg.length(), 0);

    server_history(this->clients[client_fd].get_nickname() + " invited " + target_nick + " to " + channel_name);
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
            std::string response = ":" + this->clients[client_fd].get_nickname() + " MODE " + target + " " + mode_str + "\r\n";
            send(client_fd, response.c_str(), response.length(), 0);
        }
        else
        {
            std::string response = ":" + server_name + " 221 " + this->clients[client_fd].get_nickname() + " +\r\n";
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
        std::string response = ":" + server_name + " 324 " + this->clients[client_fd].get_nickname() + " " + channel + " " + mode_response + "\r\n";
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
	for (size_t i = 0; i < receivers.size(); i++) 
	{
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
			if  (receiver_fd == -2)
				handle_bot_message(client_fd, message);
			else
			{
				std::string formatted = ":" + client_sender_nick + "!~" + this->clients[client_fd].get_username() + "@localhost PRIVMSG " + *it + " :" + message;
				send(receiver_fd, formatted.c_str(), formatted.size(), 0);
				std::cout << "Client fd: " << receiver_fd << std::endl;
				std::cout <<  "message: " << message << std::endl;
				server_history("Relaying message from " + client_sender_nick + " to " + *it);
			}
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
	
	this->clients[client_fd].pass = true;
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
	this->clients[client_fd].set_realname(realname);
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
	if (this->clients[client_fd].has_nickname() && this->clients[client_fd].is_fully_authenticated())
    {
        std::string old_prefix = get_client_prefix(this->clients[client_fd]);
        this->clients[client_fd].set_nickname(nickname);
        std::string message = ":" + old_prefix + " NICK " + nickname + "\r\n";
        send(client_fd, message.c_str(), message.length(), 0);
        std::cout << "Client " << client_fd << " changed nickname to: " << nickname << std::endl;
    }
    else
    {
        this->clients[client_fd].set_nickname(nickname);
        std::cout << "Client " << client_fd << " set nickname to: " << nickname << std::endl;
        check_registration_complete(client_fd);
	}
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
		handle_topic_delete(client_fd, iss);
	else if (is_existing_channel(arg))
		handle_topic_channel(client_fd, iss, arg);
}

void Server_class::handle_quit_command(int client_fd, std::istringstream &iss)
{
	std::string message;

	std::vector<int>::iterator client_it;
	std::vector<int>::iterator operator_it;
	std::map<std::string, Channel>::iterator channel_it;

	channel_it = this->channels.begin();
	while (channel_it != channels.end())
	{
		client_it = std::find(channel_it->second.Clients.begin(), channel_it->second.Clients.end(), client_fd);
		if (client_it != channel_it->second.Clients.end())
			channel_it->second.Clients.erase(client_it);
		
		// operator_it = std::find(channel_it->second.Operators.begin(), channel_it->second.Operators.end(), client_fd);
		// if (operator_it != channel_it->second.Operators.end())
		//     channel_it->second.Operators.erase(operator_it);
		
		channel_it++;
	}
	transfer_operator_on_disconnect(client_fd);
	
	this->clients.erase(client_fd);
	if (iss >> message)
		server_history(message);
}


void Server_class::handle_who_command(int client_fd, std::istringstream& iss)
{
    std::string target;
	std::string end_msg;
	std::string who_reply;
	int target_fd;

    iss >> target;
    std::string requester_nick = this->clients[client_fd].get_nickname();
    if (target.empty())
    {
        end_msg = ":" + server_name + " 315 " + requester_nick + " * :End of WHO list\r\n";
    	send(client_fd, end_msg.c_str(), end_msg.length(), 0);
        return;
    }
    if (target[0] == '#')
    {
        if (!is_existing_channel(target))
		{
        	return(send_error_mess(client_fd, ERR_NOSUCHCHANNEL, "No such channel", target));
		}
		std::vector<int>::iterator it;
		for (it = this->channels[target].Clients.begin(); it != this->channels[target].Clients.end(); ++it)
		{
			std::string flags = "H";
			if (is_channel_operator(*it, target))
			flags += "@";
			who_reply = ":" + server_name + " 352 " + requester_nick + " " +  target + " " + this->clients[*it].get_username() + " localhost " + server_name + " " + this->clients[*it].get_nickname() + " " + flags + " :" + this->clients[*it].get_realname() + "\r\n";
			send(client_fd, who_reply.c_str(), who_reply.length(), 0);
		}
		end_msg = ":" + server_name + " 315 " + requester_nick + " " + target + " :End of WHO list\r\n";
		send(client_fd, end_msg.c_str(), end_msg.length(), 0);
    }
    else
    {
        target_fd = is_existing_client(target);
        if (target_fd == -1)
            return(send_error_mess(client_fd, ERR_NOSUCHNICK, "No such nick", target));
		who_reply = ":" + server_name + " 352 " + requester_nick + " * " + this->clients[target_fd].get_username() + " localhost " + server_name + " " + this->clients[target_fd].get_nickname() + " H : " + this->clients[target_fd].get_realname() + "\r\n";
		send(client_fd, who_reply.c_str(), who_reply.length(), 0);
		end_msg = ":" + server_name + " 315 " + requester_nick + " " + target + " :End of WHO list\r\n";
		send(client_fd, end_msg.c_str(), end_msg.length(), 0);
    }
}

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