#include "ft_irc.hpp"

Channel::Channel()
{
	topic = "";
	created = false;
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
// 	std::vector<int>::iterator it2;
// 	std::map<std::string, Channel>::iterator it;

// 	if (this->channels[channel_name].created == true)
// 	{
// 		it = this->channels.begin();
// 		it2 = it->second.Clients.begin();
// 		while(it2 != it->second.Clients.end())
// 		{
// 			send(*it2, buffer.c_str(), buffer.size(), 0);
// 			this->server_history(client_fd + " sent a PRIVMSG to " + *it2 + '\r' + '\n');
// 			it2++;
// 		}
// 	}
// }


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
			if (*it2 != client_fd) // don't send to sender
				send(*it2, buffer.c_str(), buffer.size(), 0);
			it2++;
		}
	}
}

//this function is only to be called when said channel is SURE to be created
// bool	Channel::is_client_in_channel(int client_fd, std::string& nickname)
// {
	
// }