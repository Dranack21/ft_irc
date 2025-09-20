#include "ft_irc.hpp"

void	Server_class::send_error_message(int client_fd, std::string error_msg)
{
	std::string response = "ERROR :" + error_msg + "\r\n";
	send(client_fd, response.c_str(), response.length(), 0);
}
