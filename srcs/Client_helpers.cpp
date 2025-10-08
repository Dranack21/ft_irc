#include "ft_irc.hpp"

void	Server_class::disconnect_client(int client_fd)
{
	std::vector<pollfd>::iterator	it;
	std::map<int, Client>::iterator client_it;

	it = this->pollfd_vector.begin();
	while(it != this->pollfd_vector.end())
	{
		if (it->fd == client_fd)
		{
			this->pollfd_vector.erase(it);
			break;
		}
		it++;
	}
	client_it = this->clients.begin();
	while(client_it != this->clients.end())
	{
		if (client_it->second.get_fd() == client_fd)
		{
			this->clients.erase(client_it);
			break;
		}
		client_it++;
	}
	close(client_fd);
	std::ostringstream oss;
	oss << "Disconnected client " << client_fd;
	std::string str = oss.str();
	this->server_history(str);
}
