#include "ft_irc.hpp"

Server_class::Server_class()
{

}

Server_class::~Server_class()
{
}

void	Server_class::Setup_server(int port)
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



// This function accepts clients connections and monitors file descriptors with polling function
// pollfd struct: 
// 		-fd : a fd to monitor
//		-event : what requested event to track
//		-revent : what actually happened
//
// We first monitor the server socket with poll, if poll detects something we enter line 48 and add the new client to the vector of pollfd that we monitor
void	Server_class::Accept_and_poll()
{
	int		client_fd;
	pollfd	client;
	pollfd	server;
	char	buffer[512]; ///IRC SIZE LIMIT

	server.fd = this->Server_socket;
	server.events = POLLIN;
	server.revents = 0;
	
	this->fds.push_back(server);
	while (1)
	{
		poll(this->fds.data() , this->fds.size(), -1);
		if (this->fds[0].revents & POLLIN)
		{
			client_fd = accept(this->Server_socket , NULL, NULL);
			client.fd = client_fd;
			client.events = POLLIN;
			client.revents = 0;
			this->fds.push_back(client);
		}
		for (size_t i = 0; i < this->fds.size(); i++)
		{
			if (this->fds[i].revents & POLLIN)
			{
				if (recv(this->fds[i].fd, buffer, sizeof(buffer), 0) < 0)
					std::cout << "recv failed error code : " << errno << std::endl;
				else
					this->read_message(buffer);
			}
		}
	}
}


void	Server_class::read_message(std::string buffer)
{
	std::cout << "Message is " << buffer << std::endl;
}
