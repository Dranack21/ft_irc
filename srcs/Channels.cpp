#include "ft_irc.hpp"

Channel::Channel()
{
	topic = "";
	created = false;
}

bool	check_if_valid_channel_name(std::string name)
{
	size_t pos;

	if (name.empty() || (name[0] != '&' && name[0] != '#'))
		return false;
	pos = name.find(' ');
	if (pos != std::string::npos)
		return false;
	pos = name.find('\t');
	if (pos != std::string::npos)
		return false;
	pos = name.find('\r');
	if (pos != std::string::npos)
		return false;
	return true;
}

void Server_class::Welcome_msg_channel(int client_fd, std::string& channel_name)
{
    std::string nickname = this->clients[client_fd].get_nickname();
    
    if (!this->channels[channel_name].topic.empty())
    {
        std::string msg = ":server 332 " + nickname + " " + channel_name + " :" + this->channels[channel_name].topic + "\r\n";
        send(client_fd, msg.c_str(), msg.length(), 0);
    }
    else
    {
        std::string msg = ":server 331 " + nickname + " " + channel_name + " :No topic is set\r\n";
        send(client_fd, msg.c_str(), msg.length(), 0);
    }
    std::string names = "";
    std::vector<int>::iterator it;
    for (it = this->channels[channel_name].Clients.begin(); it != this->channels[channel_name].Clients.end(); ++it)
    {
        if (it != this->channels[channel_name].Clients.begin())
            names += " ";
        names += this->clients[*it].get_nickname();
    }
    std::string namreply = ":server 353 " + nickname + " = " + channel_name + " :" + names + "\r\n";
    send(client_fd, namreply.c_str(), namreply.length(), 0);
    std::string endnames = ":server 366 " + nickname + " " + channel_name + " :End of /NAMES list\r\n";
    send(client_fd, endnames.c_str(), endnames.length(), 0);
}

void	Server_class::Join_channel(int client_fd, std::string channel_name, std::vector<std::string> &keys)
{
	std::vector<std::string>::iterator it;

	if (this->channels.empty() || this->channels[channel_name].created == false)
	{
		this->channels[channel_name].created = true;
		this->channels[channel_name].Operators.push_back(client_fd);
		this->channels[channel_name].Clients.push_back(client_fd);
		if (keys.empty())
			this->channels[channel_name].has_password = false;
		else
		{
			this->channels[channel_name].has_password = true;
			this->channels[channel_name].password = *keys.begin();
			it = keys.begin();
			keys.erase(it);
		}
		Welcome_msg_channel(client_fd, channel_name);
	}
	else ///channel alredy e
	{
		if (!keys.empty() && this->channels[channel_name].has_password == true)
		{
			if (this->channels[channel_name].password == *keys.begin())
			{
				this->channels[channel_name].Clients.push_back(client_fd);
				Welcome_msg_channel(client_fd, channel_name);
			}
			else
				send_error_mess(client_fd, ERR_BADCHANNELKEY, "Wrong Channel Key to join: ", channel_name);
		}
		else if (keys.empty() && this->channels[channel_name].has_password == true)
		{
			send_error_mess(client_fd, ERR_NEEDMOREPARAMS, "Missing channel key to join: ", channel_name);
		}
		else if (keys.empty() && this->channels[channel_name].has_password == false)
		{
			this->channels[channel_name].Clients.push_back(client_fd);
			Welcome_msg_channel(client_fd, channel_name);
		}
	}
}


// void	Server_class::send_message_to_channel(int client_fd, const std::string &channel_name, const std::string &buffer)
// {
// 	if (this->channels[channel_name].created == false)
// 		send_error_mess(client_fd, ERR_NOSUCHCHANNEL, channel_name + ": This channel does not exist");
// 	else if (this->channels[channel_name].created)
// }

// //this function is only to be called when said channel is SURE to be created
// bool	Channel::is_client_in_channel(int client_fd, std::string& nickname)
// {
	
// }