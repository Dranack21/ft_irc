#pragma once

#include <string>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include <map>
#include <sstream>
#include <fcntl.h>
#include <poll.h>
#include <cstdlib>
#include <sys/socket.h>
#include "cstring"
#include <cerrno>

struct in_addr2
{
	in_addr_t	s_addr = INADDR_ANY;
};

struct s_a
{
	sa_family_t 		sin_family = AF_INET;	///protocol IPV4
	in_port_t			sin_port;				//16bytes int that represents port number we need to convert using htons I think
    struct	in_addr2	sin_adrr;				//32 BIT IPV4 ADRESS
};

class Client    
{
	private:
        int fd;
        std::string username;
    	std::string nickname;
		bool authenticated;
    public:
	    std::string buffer;
		Client();
		Client(int client_fd);
		Client(std::string username, std::string nickname);
		~Client();
};

class Server_class
{
	private:
		struct s_a			socket_addr;
		std::vector<pollfd>	pollfd_vector;
		std::map<int, Client> clients;
		std::string				server_password;
	public:
		int					Server_socket;
		Server_class();
		~Server_class();
		void	handle_message(int client_fd, const std::string& data);
		void	Setup_server(int port, std::string password);
		void	Accept_and_poll();
		void	process_client_activity();
		void	read_message(int client_fd, const std::string& buffer);
		void	parse_and_execute_command(int client_fd, const std::string &complete_message);
		void	send_error_message(int client_fd, std::string error_msg);
		// void	PASS_command(int);
};

// / int poll(struct pollfd *fds, nfds_t nfds, int timeout);

// The function poll takes an array of pollfd, checks each fd for the event specified in events and write what actually happened in revents
