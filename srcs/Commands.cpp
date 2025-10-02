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
	{

	}
	 else if (command == "WHO")
    {
        std::string response = ":" + server_name + " 315 " + 
                             this->clients[client_fd].get_nickname() + 
                             " * :End of WHO list\r\n";
        send(client_fd, response.c_str(), response.length(), 0);
    }
    else if (command == "PING")
    {
        std::string token;
        iss >> token;
        std::string response = ":" + server_name + " PONG " + server_name + " :" + token + "\r\n";
        send(client_fd, response.c_str(), response.length(), 0);
    }
	else
		send_error_mess(client_fd, ERR_UNKNOWNCOMMAND, "UNKNOW COMMAND");
    
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
    }
    else
    {
        // Channel modes - would need to check if user is in channel
        // For now, just send a basic error or implement channel mode logic
        send_error_mess(client_fd, ERR_UNKNOWNCOMMAND, "Channel MODE not yet implemented");
    }
}

void	Server_class::handle_priv_command(int client_fd, std::istringstream& iss)
{
	std::string					message, receivers_str, temp; 
	std::vector<std::string>	receivers;
	std::string					client_nick;

	client_nick = this->clients[client_fd].get_nickname();
	if (!(iss >> receivers_str))
	{
		send_error_mess(client_fd, ERR_NEEDMOREPARAMS, "PRIVMSG :Need at least a recipient");
		return;
	}
	if (!(iss >> message))
	{
		send_error_mess(client_fd, ERR_NEEDMOREPARAMS, "PRIVMSG :Need a message to be sent and at least a recipient");
		return ;
	}
	if (message[0] != ':')
	{
		send_error_mess(client_fd, ERR_UNKNOWNERROR, "PRIVMSG :Invalid synthax");
		return ;
	}
	receivers = Split_by_comma(receivers_str);
	while (iss >> temp)
		message += " " + temp;
	message += "\r\n";
	for(std::vector<std::string>::iterator it = receivers.begin(); it != receivers.end(); it++)
	{
		if (is_existing_receiver(*it))
		{
			if (send(client_fd, message.c_str(), message.length(), 0) != -1)
				server_history("Relaying message from " + client_nick + "to " + *it);
		}
		else
		{
			send_error_mess(client_fd, 401, client_nick + " :No such nick/channel", *it);
			server_history("Sending to " + client_nick + server_name + "401 No such nick/channel");
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
	iss >> password;
	
	if (password.empty())
	{
		send_error_mess(client_fd, ERR_NEEDMOREPARAMS, "Not enough parameters", "PASS");
		return;
	}
	
	if (this->clients[client_fd].is_authenticated())
	{
		send_error_mess(client_fd, ERR_ALREADYREGISTRED, "You may not reregister");;
		return;
	}
	
	if (password == this->server_password)
	{
		this->clients[client_fd].set_authenticated(true);
		check_registration_complete(client_fd);
	}
	else
	{
		send_error_mess(client_fd, ERR_PASSWDMISMATCH, "Password incorrect");
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
