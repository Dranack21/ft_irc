#include "ft_irc.hpp"

void Server_class::Welcome_msg_channel(int client_fd, std::string& channel_name)
{
    std::string nickname = this->clients[client_fd].get_nickname();
    std::string username = this->clients[client_fd].get_username();
    
    std::string join_broadcast = ":" + nickname + "!~" + username + "@localhost JOIN :" + channel_name + "\r\n";
    send_message_to_channel(client_fd, channel_name, join_broadcast);
    // 1. Send JOIN confirmation FIRST (critical for IRSSI)
    std::string join_msg = ":" + nickname + "!~" + username + "@localhost JOIN :" + channel_name + "\r\n";
    send(client_fd, join_msg.c_str(), join_msg.length(), 0);
    
    // 2. Send topic or no-topic message
    if (!this->channels[channel_name].topic.empty())
    {
        std::string msg = ":ft_irc.42.fr 332 " + nickname + " " + channel_name + " :" + this->channels[channel_name].topic + "\r\n";
        send(client_fd, msg.c_str(), msg.length(), 0);
    }
    else
    {
        std::string msg = ":ft_irc.42.fr 331 " + nickname + " " + channel_name + " :No topic is set\r\n";
        send(client_fd, msg.c_str(), msg.length(), 0);
    }
    
    // 3. Build and send NAMES list
    std::string names = "";
    std::vector<int>::iterator it;
    for (it = this->channels[channel_name].Clients.begin(); it != this->channels[channel_name].Clients.end(); ++it)
    {
        if (it != this->channels[channel_name].Clients.begin())
            names += " ";
        if (is_channel_operator(*it, channel_name))
            names += "@";
        names += this->clients[*it].get_nickname();
    }
    std::string namreply = ":ft_irc.42.fr 353 " + nickname + " = " + channel_name + " :" + names + "\r\n";
    send(client_fd, namreply.c_str(), namreply.length(), 0);
    
    // 4. Send end of NAMES
    std::string endnames = ":ft_irc.42.fr 366 " + nickname + " " + channel_name + " :End of /NAMES list\r\n";
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

bool	Channel::is_client_in_channel(int client_fd)
{
	std::vector<int>::iterator it;

    if (this->created == false)
        return (false);
    it = this->Clients.begin();
    while (it != this->Clients.end())
    {
        if (*it == client_fd)
            return (true);
        it++;
    }
    return (false);
}
void Server_class::broadcast_names_to_channel(const std::string& channel_name)
{
    std::vector<int>::iterator it;
    for (it = this->channels[channel_name].Clients.begin(); 
         it != this->channels[channel_name].Clients.end(); ++it)
    {
        std::string nickname = this->clients[*it].get_nickname();
        std::string names = "";
        
        // Build NAMES list with @ for operators
        std::vector<int>::iterator name_it;
        for (name_it = this->channels[channel_name].Clients.begin(); 
             name_it != this->channels[channel_name].Clients.end(); ++name_it)
        {
            if (name_it != this->channels[channel_name].Clients.begin())
                names += " ";
            
            if (is_channel_operator(*name_it, channel_name))
                names += "@";
            
            names += this->clients[*name_it].get_nickname();
        }
        
        std::string namreply = ":ft_irc.42.fr 353 " + nickname + " = " + 
                              channel_name + " :" + names + "\r\n";
        send(*it, namreply.c_str(), namreply.length(), 0);
        
        std::string endnames = ":ft_irc.42.fr 366 " + nickname + " " + 
                              channel_name + " :End of /NAMES list\r\n";
        send(*it, endnames.c_str(), endnames.length(), 0);
    }
}