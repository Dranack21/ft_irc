#include "ft_irc.hpp"
#include <map>

int main(int argc, char *argv[])
{
	Server_class	Server;
	
	if (argc != 3)
	{
		std::cout << "Need 2 args" << std::endl;
		return (1);
	}
	try{
		Server.Setup_server(atoi(argv[1]));
		Server.Accept_and_poll();
	}
	catch(const std::exception &e){
		std::cout << e.what();
		return 0;
	}
}

