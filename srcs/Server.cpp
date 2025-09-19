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
	pollfd	client_pollfd;
	pollfd	server_pollfd;

	server_pollfd.fd = this->Server_socket;
	server_pollfd.events = POLLIN;
	server_pollfd.revents = 0;
	
	this->pollfd_vector.push_back(server_pollfd);
	while (1)
	{
		poll(this->pollfd_vector.data() , this->pollfd_vector.size(), -1);
		if (this->pollfd_vector[0].revents & POLLIN)
		{
			client_fd = accept(this->Server_socket , NULL, NULL);
			client_pollfd.fd = client_fd;
			client_pollfd.events = POLLIN;
			client_pollfd.revents = 0;
			this->pollfd_vector.push_back(client_pollfd);
			this->clients[client_fd] = Client(client_fd);
                std::cout << "client connected: fd " << client_fd << std::endl;
		}
        process_client_activity();
    }
}

void Server_class::process_client_activity()
{
	int		bytes_received;
	char	buffer[512];

	for (size_t i = 1; i < this->pollfd_vector.size(); i++)
        {
            if (this->pollfd_vector[i].revents & (POLLIN | POLLHUP | POLLERR))
            {
                bytes_received = recv(this->pollfd_vector[i].fd, buffer, sizeof(buffer) - 1, 0);
                
                if (bytes_received <= 0 || this->pollfd_vector[i].revents & (POLLHUP | POLLERR)) ////peut etre handle 0 et -1 differement
                {
                    std::cout << "Client disconnected: fd " << this->pollfd_vector[i].fd << std::endl;
                    close(this->pollfd_vector[i].fd);
                    this->clients.erase(this->pollfd_vector[i].fd);
                    this->pollfd_vector.erase(this->pollfd_vector.begin() + i);
                    i--;
                }
                else
                {
                    buffer[bytes_received] = '\0';
                    //this->handle_message(this->pollfd_vector[i].fd, std::string(buffer));
					read_message(this->pollfd_vector[i].fd, std::string(buffer));
                }
            }
        }
}

void Server_class::handle_message(int client_fd, const std::string& data)
{
    this->clients[client_fd].buffer += data;

	//the thing \r\n

}


void Server_class::read_message(int client_fd, const std::string& buffer)
{
    std::cout << "Client " << client_fd << " sent: " << buffer << std::endl;
    std::cout << "Message is " << buffer << std::endl;
}