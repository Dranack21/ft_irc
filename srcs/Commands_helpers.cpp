#include "ft_irc.hpp"

bool	Server_class::is_existing_receiver(std::string &receiver)
{
	std::map<int, Client>::iterator it;
	std::map<std::string, Channel>::iterator channel_it;
	std::string		username;
	std::string		channels;				

	it = this->clients.begin();
	while (it != this->clients.end())
	{
		username = it->second.get_nickname();
		if (username == receiver)
			return (true);
		it++;
	}
	channel_it = this->channels.begin();
	while (channel_it != this->channels.end())
	{
		channels = channel_it->second.name;
		if (channels == receiver)
			return (true);
		channel_it++;
	}
	return false;
}

int	Server_class::is_existing_client(std::string &receiver)
{
	std::map<int, Client>::iterator it;
	std::string		nickname;

	it = this->clients.begin();
	while (it != this->clients.end())
	{
		nickname = it->second.get_nickname();
		if (nickname == receiver)
			return (it->second.get_fd());
		it++;
	}
	return (-1);
}
bool Server_class::is_existing_channel(const std::string &receiver)
{
    std::map<std::string, Channel>::iterator it = this->channels.find(receiver);
    return (it != this->channels.end() && it->second.created);
}

bool Server_class::is_channel_operator(int client_fd, const std::string& channel)
{
    std::vector<int>::iterator it;
    for (it = this->channels[channel].Operators.begin(); 
         it != this->channels[channel].Operators.end(); ++it)
    {
        if (*it == client_fd)
            return true;
    }
    return false;
}

std::string Server_class::build_channel_mode_string(const std::string& channel)
{
    std::string modes = "+";
    std::string params = "";
    
    // Add 't' if topic is restricted (default)
    if (this->channels[channel].topic_restricted)
        modes += "t";
    
    // Add 'k' if channel has password
    if (this->channels[channel].has_password)
    {
        modes += "k";
        params += " " + this->channels[channel].password;
    }
    
    // If no modes are set, just return "+"
    if (modes == "+")
        return "+";
    
    return modes + params;
}

void	Server_class::handle_topic_delete(int client_fd, std::istringstream& iss)
{
	std::string channel_name;
///GERER TOPIC DELETE SI TOPIC EXISTE PAS
	if (!(iss >> channel_name))
		send_error_mess(client_fd, 461, "TOPIC :-delete must be followed up with a channel");
	else if (channel_name[0] != '#' || is_existing_channel(channel_name) == false|| this->channels[channel_name].is_client_in_channel(client_fd) == true)
		send_error_mess(client_fd, 403, channel_name + " :No such channel");
	else if (this->channels[channel_name].topic_restricted == true && this->channels[channel_name].is_client_operator(client_fd) == false)
		return(send_error_mess(client_fd, ERR_CHANOPRIVSNEEDED, "You're not channel operator", channel_name));
	this->channels[channel_name].topic.clear();
}
void	Server_class::handle_topic_channel(int client_fd, std::istringstream& iss, const std::string &channel_name)
{
	std::string arg;
	std::string topic;

	if (iss >> arg)
	{
		if (arg[0] == ':' && arg.length() > 1)
			topic += arg.substr(1, arg.length() -1);
		else
			topic += arg;
	}
	else
	{
		if (this->channels[channel_name].is_client_in_channel(client_fd))
			return (send_error_mess(client_fd, 332, channel_name + topic));
		else
			return (send_error_mess(client_fd, 442, channel_name + ": You're not on that channel"));
	}
	while (iss >> arg)
		topic += arg;
	if (this->channels[channel_name].topic_restricted && this->channels[channel_name].is_client_operator(client_fd) == false)
		return (send_error_mess(client_fd, ERR_CHANOPRIVSNEEDED, "You're not channel operator", channel_name));
	this->channels[channel_name].topic = topic;
	std::cout << "TOPIC :" << topic << std::endl;
	std::string response = ":" + this->clients[client_fd].get_nickname() + "!" + this->clients[client_fd].get_username() + "@" + server_name + " TOPIC " + channel_name + " :" + topic + "\r\n";
	std::cout << response << std::endl;
	send(client_fd, response.c_str(), response.length(), 0);
}
