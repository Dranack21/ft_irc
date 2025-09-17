#include "ft_irc.hpp"
#include <map>

int main(int argc, char *argv[])
{
	Server_class	Server;
	int				client;
	
	if (argc != 3)
		std::cout << "Need 2 args" << std::endl;

	Server.Setup_server(atoi(argv[1]));
	while (1)
	{
		client = accept(Server.Server_socket , NULL, NULL);
	}
	close (client);
}

