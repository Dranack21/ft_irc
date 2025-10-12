#include "ft_irc.hpp"

Channel::Channel()
{
	topic = "";
	created = false;
	has_password = false;
	topic_restricted = true;
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

void	Server_class::send_message_to_channel(int client_fd, const std::string &channel_name, const std::string &buffer)
{
	std::vector<int>::iterator it2;

	if (this->channels[channel_name].created == false)
		send_error_mess(client_fd, ERR_NOSUCHCHANNEL, channel_name + ": This channel does not exist");
	else if (this->channels[channel_name].created == true)
	{
		it2 = this->channels[channel_name].Clients.begin();
		while(it2 != this->channels[channel_name].Clients.end())
		{
			if (*it2 != client_fd)
			{
				send(*it2, buffer.c_str(), buffer.size(), 0);
				if (buffer.find(" MODE ") != std::string::npos || buffer.find(" JOIN ") != std::string::npos)
				{
					send_names_list(*it2, channel_name);
				}
			}
			it2++;
		}
	}
}

void Server_class::send_names_list(int client_fd, const std::string& channel_name)
{
	std::string nickname = this->clients[client_fd].get_nickname();
	std::string names = "";
	std::vector<int>::iterator it;
	
	for (it = this->channels[channel_name].Clients.begin(); 
	     it != this->channels[channel_name].Clients.end(); ++it)
	{
		if (it != this->channels[channel_name].Clients.begin())
			names += " ";
		if (is_channel_operator(*it, channel_name))
			names += "@";
		names += this->clients[*it].get_nickname();
	}
	
	std::string namreply = ":ft_irc.42.fr 353 " + nickname + " = " + 
	                       channel_name + " :" + names + "\r\n";
	send(client_fd, namreply.c_str(), namreply.length(), 0);
	
	std::string endnames = ":ft_irc.42.fr 366 " + nickname + " " + 
	                       channel_name + " :End of /NAMES list\r\n";
	send(client_fd, endnames.c_str(), endnames.length(), 0);
}

//this function is only to be called when said channel is SURE to be created
// bool	Channel::is_client_in_channel(int client_fd, std::string& nickname)
// {
	
// }