#include "ft_irc.hpp"

bool	Server_class::is_existing_receiver(std::string &receiver)
{
	std::map<int, Client>::iterator it;
	std::map<std::string, Channel>::iterator channel_it;
	std::string		username;
	std::string		channels;				

	it = this->clients.begin();
	while (it != this->clients.end())
	{
		username = it->second.get_nickname();
		if (username == receiver)
			return (true);
		it++;
	}
	channel_it = this->channels.begin();
	while (channel_it != this->channels.end())
	{
		channels = channel_it->second.name;
		if (channels == receiver)
			return (true);
		channel_it++;
	}
	return false;
}

int	Server_class::is_existing_client(std::string &receiver)
{
	std::map<int, Client>::iterator it;
	std::string		nickname;

	it = this->clients.begin();
	while (it != this->clients.end())
	{
		nickname = it->second.get_nickname();
		if (nickname == receiver)
			return (it->second.get_fd());
		it++;
	}
	return (-1);
}
bool Server_class::is_existing_channel(std::string &receiver)
{
    std::map<std::string, Channel>::iterator it = this->channels.find(receiver);
    return (it != this->channels.end() && it->second.created);
}

