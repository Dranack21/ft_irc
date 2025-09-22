#include "ft_irc.hpp"

void Server_class::handle_pass_command(int client_fd, std::istringstream& iss)
{
	std::string password;
	iss >> password;
	
	if (password.empty())
	{
		send_error_message(client_fd, "Not enough parameters");
		return;
	}
	
	if (this->clients[client_fd].is_authenticated())
	{
		send_error_message(client_fd, "You may not reregister");
		return;
	}
	
	if (password == this->server_password)
	{
		this->clients[client_fd].set_authenticated(true);
		check_registration_complete(client_fd);
	}
	else
	{
		send_error_message(client_fd, "Password incorrect");
	}
}

void Server_class::handle_user_command(int client_fd, std::istringstream& iss)
{
	std::string username, hostname, servername, realname;
	iss >> username >> hostname >> servername;
	
	std::string remaining;
	std::getline(iss, remaining);
	if (!remaining.empty() && remaining[0] == ' ')
		remaining = remaining.substr(1);
	if (!remaining.empty() && remaining[0] == ':')
		realname = remaining.substr(1);
	else
		realname = remaining;
	
	if (username.empty())
	{
		send_error_message(client_fd, "Not enough parameters");
		return;
	}
	
	if (this->clients[client_fd].has_username())
	{
		send_error_message(client_fd, "You may not reregister");
		return;
	}
	
	this->clients[client_fd].set_realname(realname); //forgor to set realname
	this->clients[client_fd].set_username(username);
	std::cout << "Client " << client_fd << " set username to: " << username << std::endl;

	check_registration_complete(client_fd);
}

void Server_class::handle_nick_command(int client_fd, std::istringstream& iss)
{
	std::string nickname;
	iss >> nickname;
	
	if (nickname.empty())
	{
		send_error_message(client_fd, "No nickname given");
		return;
	}
	
	if (is_nickname_in_use(nickname))
	{
		send_error_message(client_fd,"Nickname is already in use");
		return;
	}
	
	if (!is_valid_nickname(nickname))
	{
		send_error_message(client_fd, "Weird nickname");
		return;
	}
	if (this->clients[client_fd].has_nickname() && this->clients[client_fd].is_fully_authenticated()) // if already has a nick so is to change it 
	{
		std::string old_prefix = get_client_prefix(this->clients[client_fd]);
		this->clients[client_fd].set_nickname(nickname);
		std::string message = ":" + old_prefix + " NICK " + nickname + "\r\n";
		send(client_fd, message.c_str(), message.length(), 0);
	}
	else
	{
		this->clients[client_fd].set_nickname(nickname);
	}
	std::cout << "Client " << client_fd << " set nickname to: " << nickname << std::endl;
	check_registration_complete(client_fd);
}
