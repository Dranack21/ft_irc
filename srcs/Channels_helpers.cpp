#include "ft_irc.hpp"

void Server_class::Welcome_msg_channel(int client_fd, std::string& channel_name)
{
    std::string nickname = this->clients[client_fd].get_nickname();
    
    if (!this->channels[channel_name].topic.empty())
    {
        std::string msg = ":server 332 " + nickname + " " + channel_name + " :" + this->channels[channel_name].topic + "\r\n";
        send(client_fd, msg.c_str(), msg.length(), 0);
    }
    else
    {
        std::string msg = ":server 331 " + nickname + " " + channel_name + " :No topic is set\r\n";
        send(client_fd, msg.c_str(), msg.length(), 0);
    }
    std::string names = "";
    std::vector<int>::iterator it;
    for (it = this->channels[channel_name].Clients.begin(); it != this->channels[channel_name].Clients.end(); ++it)
    {
        if (it != this->channels[channel_name].Clients.begin())
            names += " ";
        names += this->clients[*it].get_nickname();
    }
    std::string namreply = ":server 353 " + nickname + " = " + channel_name + " :" + names + "\r\n";
    send(client_fd, namreply.c_str(), namreply.length(), 0);
    std::string endnames = ":server 366 " + nickname + " " + channel_name + " :End of /NAMES list\r\n";
    send(client_fd, endnames.c_str(), endnames.length(), 0);
}

bool	check_if_valid_channel_name(std::string name)
{
	size_t pos;

	if (name.empty() || (name[0] != '&' && name[0] != '#'))
		return false;
	pos = name.find(' ');
	if (pos != std::string::npos)
		return false;
	pos = name.find('\t');
	if (pos != std::string::npos)
		return false;
	pos = name.find('\r');
	if (pos != std::string::npos)
		return false;
	return true;
}

