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
#include <signal.h>
#include <csignal>
#include <cstdio>
#include <algorithm>


#define RPL_WELCOME 001
#define RPL_YOURHOST 002
#define RPL_CREATED 003
#define RPL_MYINFO 004


#define RLP_WHOISUSER 311
#define RPL_WHOISSERVER 312
#define RPL_WHOISOPERATOR 313
#define RPL_WHOWASUSER 314
#define RPL_ENDOFWHO 315
#define RPL_WHOISCHANNELS 320
#define RPL_NOTOPIC 331
#define RPL_TOPIC 332
#define RPL_WHOREPLY 352
#define RPL_NAMREPLY 353
#define RPL_ENDOFNAMES 366

#define ERR_UNKNOWNERROR 400
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
#define ERR_USERNOTINCHANNEL    441
#define ERR_NOTONCHANNEL        442
#define ERR_USERONCHANNEL       443
#define ERR_NEEDMOREPARAMS 461
#define ERR_ALREADYREGISTRED 462
#define ERR_PASSWDMISMATCH 464
#define ERR_CHANNELISFULL       471
#define ERR_UNKNOWNMODE         472
#define ERR_INVITEONLYCHAN      473
#define ERR_BADCHANNELKEY 475
#define ERR_CHANOPRIVSNEEDED    482


struct in_addr2
{
	in_addr_t	s_addr;
};
struct s_a
{
	sa_family_t 		sin_family;	///protocol IPV4
	in_port_t			sin_port;				//16bytes int that represents port number we need to convert using htons I think
    struct	in_addr2	sin_addr;				//32 BIT IPV4 ADRESS
	char             sin_zero[8];
};



struct Channel
{
	Channel();
	bool		created;
	std::string name;
	std::string topic;
	std::string password;
	bool	has_password;
	bool	topic_restricted; //if true only ops can change topic
	std::vector<int> Clients;	//vecteur d'int contenant les FD des clients 
	std::vector<int> Operators; //vecteur d'int contenant les FD des operateurs
	bool						is_client_in_channel(int client_fd);
	bool						is_client_operator(int client_fd);
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
		int			last_client;
	    std::string buffer;
		pollfd		pollfd_copy;
		Client();
		Client(int client_fd);
		Client(std::string username, std::string nickname);
		~Client();

		int get_fd() const { return fd; }
		std::string	get_username() const { return username; }
		std::string	get_nickname() const { return nickname; }
		std::string	get_realname() const { return realname; }
		bool	is_authenticated() const { return authenticated; }
		bool	has_username() const { return has_user; }
		bool	has_nickname() const { return has_nick; }
		bool	is_fully_authenticated() const { return authenticated && has_nick && has_user; }

		void	set_realname(const std::string& real);
		void	set_username(const std::string& user);
		void	set_nickname(const std::string& nick);
		void	set_authenticated(bool auth);
};

class Server_class
{
	private:
		struct sockaddr_in socket_addr;
		std::vector<pollfd>		pollfd_vector;
		std::map<std::string, Channel> channels;
		std::map<int, Client>	clients;
		std::string				server_password;
		std::string				server_name;
		std::string				server_version;
		std::string				creation_date;
		static Server_class* 	instance;
		bool running;
	public:
		int					Server_socket;
		Server_class();
		~Server_class();
		void	handle_message(int client_fd, const std::string& data);
		void	Setup_server(int port, std::string password);
		void	Accept_and_poll();
		void	process_client_activity();

		void	server_history(const std::string &buffer);
		void	read_message(int client_fd, const std::string& buffer);
		void	parse_for_register(int client_fd, const std::string &complete_message);
		void	parse_and_execute_command(int client_fd, const std::string &complete_message);
		void	send_error_message(int client_fd, std::string error_msg);
		// void	PASS_command(int);


		void 	send_error_mess(int client_fd, int numeric, const std::string& message, const std::string& target = "");
		void	send_welcome_sequence(int client_fd);


		void	handle_pass_command(int client_fd, std::istringstream& iss);
		void	handle_nick_command(int client_fd, std::istringstream& iss);
		void	handle_user_command(int client_fd, std::istringstream& iss);
		void	handle_join_command(int client_fd, std::istringstream& iss);
		void	handle_priv_command(int client_fd, std::istringstream& iss);
		void	handle_mode_command(int client_fd, std::istringstream& iss);
		void	handle_ping_command(int client_fd, std::istringstream& iss);
		void	handle_whois_command(int client_fd, std::istringstream& iss);
		void	handle_topic_command(int client_fd, std::istringstream& iss);
		void	handle_quit_command(int client_fd, std::istringstream& iss);
		void	handle_who_command(int client_fd, std::istringstream& iss);
		void	end_of_whois(int client_fd);
	

		void	check_registration_complete(int client_fd);
		bool	is_nickname_in_use(const std::string& nickname);
		bool	is_valid_nickname(const std::string& nickname);

		std::string	get_client_prefix(const Client& client);
		std::string	to_upper(const std::string& str);

		////CHANNELS
		void						Join_channel(int client_fd, std::string channel_name, std::vector<std::string> &keys);
		void						send_message_to_channel(int client_fd,const std::string &channel,  const std::string &buffer);
		void						Welcome_msg_channel(int client_fd, std::string& channel_name);
		std::vector<std::string>	Split_by_comma(std::string &channels);
		int							get_fd_from_nick(std::string nick);
		bool						is_channel_operator(int client_fd, const std::string& channel);
		std::string					build_channel_mode_string(const std::string& channel);
		void						apply_channel_modes(int client_fd, const std::string& channel, const std::string& mode_string, std::istringstream& iss);
		void						send_names_list(int client_fd, const std::string& channel_name);
		void						broadcast_names_to_channel(const std::string& channel_name);
		bool 						process_single_mode(int client_fd, const std::string& channel, char mode, bool adding, std::istringstream& iss, std::string& param);
		bool						handle_operator_mode(int client_fd, const std::string& channel, bool adding, std::istringstream& iss, std::string& param);
		bool 						handle_key_mode(int client_fd, const std::string& channel, bool adding, std::istringstream& iss,std::string& param);
		bool 						handle_topic_mode(int client_fd, const std::string& channel, bool adding);
		void						remove_operator_status(int target_fd, const std::string& channel);
		void						broadcast_mode_changes(int client_fd, const std::string& channel, const std::string& applied_modes, const std::string& applied_params);
		void						transfer_operator_on_disconnect(int disconnecting_fd);

		////PRIVMSG
		bool	is_existing_receiver(std::string &receiver);
		bool	is_existing_channel(const std::string &receiver);
		int		is_existing_client(std::string &receiver);

		///TOPIC 
		void	handle_topic_delete(int client_fd, std::istringstream& iss);
		void	handle_topic_channel(int client_fd, std::istringstream& iss, const std::string &channel_name);
		static void signal_handler(int signum);
		void		disconnect_client(int client_fd);
		void 		shutdown_server();
};

// / int poll(struct pollfd *fds, nfds_t nfds, int timeout);

// The function poll takes an array of pollfd, checks each fd for the event specified in events and write what actually happened in revents

bool	check_if_valid_channel_name(std::string name);