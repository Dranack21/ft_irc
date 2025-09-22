#include "ft_irc.hpp"

void	Server_class::send_error_message(int client_fd, std::string error_msg)
{
	std::string response = "ERROR :" + error_msg + "\r\n";
	send(client_fd, response.c_str(), response.length(), 0);
}

//error function that makes an output like :
// :<server_name> <numeric> <nickname or *> <target> :<message>
//or
// :<server_name> <numeric> <nickname or *> :<message> 
//if no target provided 
void Server_class::send_error_mess(int client_fd, int numeric, const std::string& message, const std::string& target)  //send error message ++ have to check if it doesn't break anything if we replace
{
	Client& client = this->clients[client_fd];
	std::string nickname = client.has_nickname() ? client.get_nickname() : "*";
	
	std::ostringstream oss;
	oss << ":" << server_name << " ";
	oss.fill('0');
	oss.width(3);
	oss << numeric << " " << nickname;
	
	if (!target.empty())
		oss << " " << target;
		
	oss << " :" << message << "\r\n";
	
	std::string response = oss.str();
	send(client_fd, response.c_str(), response.length(), 0);
}


void Server_class::check_registration_complete(int client_fd)
{
	Client& client = this->clients[client_fd];
	
	if (client.is_fully_authenticated())
	{
		std::cout << "Client " << client_fd << " (" << client.get_nickname() << ") fully registered!" << std::endl;
		send_welcome_sequence(client_fd);
	}
}

bool Server_class::is_nickname_in_use(const std::string& nickname)
{
	std::map<int, Client>::iterator it;
	for (it = this->clients.begin(); it != this->clients.end(); ++it)
	{
		if (it->second.get_nickname() == nickname)
			return true;
	}
	return false;
}

bool Server_class::is_valid_nickname(const std::string& nickname)
{
	if (nickname.empty() || nickname.length() > 9)
		return false;

	if (!std::isalpha(nickname[0]))
		return false;

	for (size_t i = 1; i < nickname.length(); i++)
	{
		char c = nickname[i];
		if (!std::isalnum(c) && c != '-' && c != '_' && c != '[' && c != ']' && c != '{' && c != '}' && c != '\\' && c != '`' && c != '^')
			return false;
	}
	return true;
}


void	Server_class::read_message(int client_fd, const std::string& buffer)
{
    std::cout << "Client " << client_fd << " sent: " << buffer << std::endl;
}

std::string Server_class::get_client_prefix(const Client& client) //function to get the prefix of a client like nickname!username@hostname we need this for messages
{
	return client.get_nickname() + "!" + client.get_username() + "@" + server_name;
}

std::string Server_class::to_upper(const std::string& str)
{
	std::string result = str;
	for (size_t i = 0; i < result.length(); i++)
	{
		if (result[i] >= 'a' && result[i] <= 'z')
			result[i] = result[i] - 'a' + 'A';
	}
	return result;
}

void Client::set_realname(const std::string& real) //temporary 
{
	this->realname = real;
}
