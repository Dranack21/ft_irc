#include "ft_irc.hpp"

Server_class* Server_class::instance = NULL; //needed for signal handling

// void	Server_class::send_error_message(int client_fd, std::string error_msg)
// {
// 	std::string response = "ERROR :" + error_msg + "\r\n";
// 	send(client_fd, response.c_str(), response.length(), 0);
// }




void	Server_class::send_welcome_sequence(int client_fd)//sends the welcome messages after registration is complete (error in send_error_mess isn't a fitting name here)
{
	Client& client = this->clients[client_fd];
	std::string nickname = client.get_nickname();
	std::string username = client.get_username();

	std::string welcome_msg = "Welcome to the " + server_name + " Network, " + nickname + "." + username + "@" + server_name;
	send_error_mess(client_fd, RPL_WELCOME, welcome_msg);

	std::string host_msg = "Your host is " + server_name + ", running version " + server_version;
	send_error_mess(client_fd, RPL_YOURHOST, host_msg);

	std::string created_msg = "This server was created " + creation_date;
	send_error_mess(client_fd, RPL_CREATED, created_msg);

	std::string info_msg = server_name + " " + server_version + " o o";
	send_error_mess(client_fd, RPL_MYINFO, info_msg);
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
	std::cout << response << std::endl;
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
    std::time_t now = std::time(0);
    char* timestr = std::ctime(&now);
	if (buffer.find("PING") == std::string::npos)
	{
		std::cout << "[" << std::string(timestr).substr(0, 24) << "] " 
		<< "Client " << client_fd << " sent: " << buffer << std::endl;
	}
}

void	Server_class::server_history(const std::string& buffer)
{
	std::time_t now = std::time(0);
    char* timestr = std::ctime(&now);

	if (buffer.find("PING") == std::string::npos)
		std::cout << "[" << std::string(timestr).substr(0, 24) << "] " << buffer << std::endl;
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

void Server_class::signal_handler(int signum)
{
    std::cout << "\nReceived signal " << signum << ". Shutting down server..." << std::endl;
    if (instance)
    {
        instance->running = false;
    }
}

void Server_class::shutdown_server() //function to shutdown the server to not get binding fails after a ctrl+c
{
    std::cout << "Shutting down server..." << std::endl;
    std::string quit_message = "ERROR :Server shutting down\r\n";
    
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        send(it->first, quit_message.c_str(), quit_message.length(), 0);
        close(it->first);
        std::cout << "Closed connection to client fd " << it->first << std::endl;
    }
    clients.clear();
	channels.clear();
    pollfd_vector.clear();
    if (Server_socket >= 0)
    {
        close(Server_socket);
        std::cout << "Server socket closed" << std::endl;
    }
    std::cout << "Server shutdown complete." << std::endl;
}

std::vector<std::string> Server_class::Split_by_comma(std::string &channels_str)
{
	size_t	pos;
	size_t	old_pos = 0;
	std::string substr;
	std::vector<std::string> channels_vector;

	pos = channels_str.find(",");
	while (pos != std::string::npos)
	{
		substr = channels_str.substr(old_pos, pos - old_pos);
		if (!substr.empty())
			channels_vector.push_back(substr);
		old_pos = pos + 1;
		pos = channels_str.find(",", old_pos);
	}
	substr = channels_str.substr(old_pos);
	if (!substr.empty())
		channels_vector.push_back(substr);
	return channels_vector;
}

void Server_class::handle_ping_command(int client_fd, std::istringstream& iss)
{
	std::string token;
	std::string response;

    iss >> token;
    response = ":" + server_name + " PONG " + server_name + " :" + token + "\r\n";
    send(client_fd, response.c_str(), response.length(), 0);
}

void Server_class::transfer_operator_on_disconnect(int disconnecting_fd)
{
    std::cout << "DEBUG: transfer_operator_on_disconnect called for fd " << disconnecting_fd << std::endl;
    std::vector<int>::iterator op_it;
    std::map<std::string, Channel>::iterator chan_it;
    
    for (chan_it = this->channels.begin(); chan_it != this->channels.end(); ++chan_it)
    {
        std::cout << "DEBUG: Checking channel " << chan_it->first << std::endl;
		
        if (!is_channel_operator(disconnecting_fd, chan_it->first))
        {
            std::cout << "DEBUG: fd " << disconnecting_fd << " is NOT an operator in " << chan_it->first << std::endl;

            continue;
        }
		
        std::cout << "DEBUG: fd " << disconnecting_fd << " IS an operator in " << chan_it->first << std::endl;

        for (op_it = chan_it->second.Operators.begin(); op_it != chan_it->second.Operators.end(); ++op_it)
        {
            if (*op_it == disconnecting_fd)
            {
                chan_it->second.Operators.erase(op_it);

                std::cout << "DEBUG: Removed fd " << disconnecting_fd << " from operators" << std::endl;

                break;
            }
        }

        std::cout << "DEBUG: Operators left: " << chan_it->second.Operators.size() << std::endl;
        std::cout << "DEBUG: Clients left: " << chan_it->second.Clients.size() << std::endl;

        if (chan_it->second.Operators.empty() && !chan_it->second.Clients.empty())
        {
            int new_op_fd = chan_it->second.Clients[0];

            std::cout << "DEBUG: Promoting fd " << new_op_fd << " to operator" << std::endl;

            chan_it->second.Operators.push_back(new_op_fd);
            std::string mode_msg = ":" + server_name + " MODE " + chan_it->first + " +o " + this->clients[new_op_fd].get_nickname() + "\r\n";

            std::cout << "DEBUG: Sending mode message: " << mode_msg;

            std::vector<int>::iterator client_it;
            for (client_it = chan_it->second.Clients.begin(); client_it != chan_it->second.Clients.end(); ++client_it)
            {
                send(*client_it, mode_msg.c_str(), mode_msg.length(), 0);
            }
            std::cout << "Transferred operator status in " << chan_it->first << " to " << this->clients[new_op_fd].get_nickname() << std::endl;
        }
    }
}