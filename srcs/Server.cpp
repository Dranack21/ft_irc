#include "ft_irc.hpp"

Server_class::Server_class()
{

}

Server_class::~Server_class()
{
}

void	Server_class::Setup_server(int port, std::string password)
{
	this->Server_socket = socket(AF_INET ,SOCK_STREAM, 0);
	this->server_password = password;
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

// This function goes thorugh a vector of pollfd more specifically each representing a client connected to our server (index 0)
// It looks for messaged receveid with revent since we use the function poll before entering this function
// Then if a message is received we use the function recv and handle it correctly 
void Server_class::process_client_activity()
{
	int		bytes_received;
	char	buffer[512];

	for (size_t i = 1; i < this->pollfd_vector.size(); i++)
        {
            if (this->pollfd_vector[i].revents & (POLLIN | POLLHUP | POLLERR))
            {
				memset(buffer, 0, sizeof(buffer));
                bytes_received = recv(this->pollfd_vector[i].fd, buffer, sizeof(buffer) - 1, 0);
                std::cout << bytes_received << std::endl;
                if (bytes_received <= 0 || this->pollfd_vector[i].revents & (POLLHUP | POLLERR)) ////peut etre handle 0 et -1 differement
                {
                    std::cout << "Client disconnected: fd " << this->pollfd_vector[i].fd << std::endl;
                    close(this->pollfd_vector[i].fd);
                    this->clients.erase(this->pollfd_vector[i].fd); //// THIS IS A MAP SO WE ERASE USING THE KEY WHICH IS THE FD OF THE CLIENT
                    this->pollfd_vector.erase(this->pollfd_vector.begin() + i);
                    i--;
                }
                else
                {
                    this->handle_message(this->pollfd_vector[i].fd, std::string(buffer));
                }
            }
        }
}

//This function takes as argument the fd of a client and its message received by recv function
// The message can be splited up so we look for \r\n which indicates the end of a message
// if Not found we keep going
void	Server_class::handle_message(int client_fd, const std::string& data)
{
	size_t		pos;
	std::string	complete_message;

    this->clients[client_fd].buffer += data;
	pos = this->clients[client_fd].buffer.find("\r\n");
	while ((pos = this->clients[client_fd].buffer.find("\r\n") )!= std::string::npos)
	{
		complete_message = this->clients[client_fd].buffer.substr(0, pos);
		
		this->clients[client_fd].buffer.erase(0, pos + 2);
		read_message(client_fd, complete_message);
		parse_and_execute_command(client_fd, complete_message);
	}
}

void	Server_class::parse_and_execute_command(int client_fd, const std::string& complete_message)
{
	std::istringstream iss(complete_message);
    std::string command;
	std::string password;
    iss >> command;
	if (command == "PASS")
	{
		iss >> password;
		if (password == this->server_password)
		{
			std::cout << "bravo tu est authentifie mtn je vais aller manger";
			(void)client_fd;
		}
		else
			send_error_message(client_fd, "Failed authentication, Incorrect Password");
	}
	else
		send_error_message(client_fd, "You must authenticate with PASS <password> first");
}

void	Server_class::read_message(int client_fd, const std::string& buffer)
{
    std::cout << "Client " << client_fd << " sent: " << buffer << std::endl;
}