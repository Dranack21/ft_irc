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
		bool has_nick;
		bool has_user;

    public:
	    std::string buffer;
		Client();
		Client(int client_fd);
		Client(std::string username, std::string nickname);
		~Client();

		int get_fd() const { return fd; }
		std::string get_username() const { return username; }
		std::string get_nickname() const { return nickname; }
		bool is_authenticated() const { return authenticated; }
		bool has_username() const { return has_user; }
		bool has_nickname() const { return has_nick; }
		bool is_fully_authenticated() const { return authenticated && has_nick && has_user; }
		

		void set_username(const std::string& user);
		void set_nickname(const std::string& nick);
		void set_authenticated(bool auth);
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

		void	handle_pass_command(int client_fd, std::istringstream& iss);
		void	handle_nick_command(int client_fd, std::istringstream& iss);
		void	handle_user_command(int client_fd, std::istringstream& iss);
		
		void	check_registration_complete(int client_fd);
		bool	is_nickname_in_use(const std::string& nickname);
		bool	is_valid_nickname(const std::string& nickname);
};

// / int poll(struct pollfd *fds, nfds_t nfds, int timeout);

// The function poll takes an array of pollfd, checks each fd for the event specified in events and write what actually happened in revents
