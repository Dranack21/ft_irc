#include "ft_irc.hpp"
#include <map>

int main(int argc, char *argv[])
{
	Server_class	Server;
	std::string 	pass;
	if (argc != 3)
	{
		std::cout << "Need 2 args" << std::endl;
		return (1);
	}
	pass = argv[2];
	if(pass.empty())
	{
		std::cout << "Empty pass is not valid" << std::endl;
		return (1);
	}
	if(atoi(argv[1]) < 1024)
	{
		std::cout << "Please use a port above 1024" << std::endl;
		return (1);
	}
	try{
		Server.Setup_server(atoi(argv[1]), argv[2]);
		Server.Accept_and_poll();
	}
	catch(const std::exception &e){
		std::cout << e.what();
		return 0;
	}
}

