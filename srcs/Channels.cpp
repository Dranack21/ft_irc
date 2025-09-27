#include "ft_irc.hpp"

bool	check_if_valid_channel_name(std::string name)
{
	size_t pos;

	if (name.empty() || name[0] != '&' || name[0] != '#')
		return false;
	pos = name.find(' ');
	if (pos != std::string::npos)
		return false;
	pos = name.find('\t');
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

void	Server_class::Join_channel(int client_fd, std::string channel_name, std::vector<std::string> &keys)
{
	std::vector<std::string>::iterator it;

	if (this->channels.empty() || this->channels[channel_name].created == false)
	{
		///WE CREATE IT OURSELVES
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
	}
	else
	{
		////CHANNEL ALREDYE EXISTS
		if (!keys.empty() && this->channels[channel_name].has_password == true)
		{
			if (this->channels[channel_name].password == *keys.begin())
				this->channels[channel_name].Clients.push_back(client_fd);
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
		}
	}
}
