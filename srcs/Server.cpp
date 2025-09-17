#include "ft_irc.hpp"

Server_class::Server_class()
{

}

Server_class::~Server_class()
{
	
}

void Server_class::Setup_server(int port)
{
	this->Server_socket = socket(AF_INET ,SOCK_STREAM, 0);
	this->socket_addr.sin_port = htons(port);
	if (fcntl(this->Server_socket, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("Fcntl flag set failed");
	if (bind(this->Server_socket, (struct sockaddr *)&socket_addr, sizeof(sockaddr)) < 0)
		throw std::runtime_error ("bind failed");
	if (listen(this->Server_socket, 50) < 0)
		throw std::runtime_error ("Listen failed");
}