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
#include <ctime>
#include "cstring"
#include <cerrno>


#define RPL_WELCOME 001
#define RPL_YOURHOST 002
#define RPL_CREATED 003
#define RPL_MYINFO 004

#define ERR_NOSUCHNICK 401
#define ERR_NOSUCHCHANNEL 403
#define ERR_CANNOTSENDTOCHAN 404
#define ERR_NORECIPIENT 411
#define ERR_NOTEXTTOSEND 412
#define ERR_UNKNOWNCOMMAND 421
#define ERR_NOMOTD 422
#define ERR_NONICKNAMEGIVEN 431
#define ERR_ERRONEUSNICKNAME 432
#define ERR_NICKNAMEINUSE 433
#define ERR_NEEDMOREPARAMS 461
#define ERR_ALREADYREGISTRED 462
#define ERR_PASSWDMISMATCH 464


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
		std::string realname;
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
		std::string get_realname() const { return realname; }
		bool is_authenticated() const { return authenticated; }
		bool has_username() const { return has_user; }
		bool has_nickname() const { return has_nick; }
		bool is_fully_authenticated() const { return authenticated && has_nick && has_user; }

		void set_realname(const std::string& real);
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
		std::string				server_name;
		std::string				server_version;
		std::string				creation_date;
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


		void 	send_error_mess(int client_fd, int numeric, const std::string& message, const std::string& target = "");
		void	send_welcome_sequence(int client_fd);


		void	handle_pass_command(int client_fd, std::istringstream& iss);
		void	handle_nick_command(int client_fd, std::istringstream& iss);
		void	handle_user_command(int client_fd, std::istringstream& iss);
		
		void	check_registration_complete(int client_fd);
		bool	is_nickname_in_use(const std::string& nickname);
		bool	is_valid_nickname(const std::string& nickname);

		std::string	get_client_prefix(const Client& client);
		std::string	to_upper(const std::string& str);
};

// / int poll(struct pollfd *fds, nfds_t nfds, int timeout);

// The function poll takes an array of pollfd, checks each fd for the event specified in events and write what actually happened in revents
