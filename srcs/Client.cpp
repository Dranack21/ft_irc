#include "ft_irc.hpp"

Client::Client() :fd(-1), authenticated(false), has_nick(false), has_user(false), pass(false)
{ 
}

Client::Client(int client_fd) : fd(client_fd), authenticated(false), has_nick(false), has_user(false), pass(false), last_client(-1)
{

}
Client::Client(std::string username, std::string nickname) :fd(-1), username(username), nickname(nickname), authenticated(false), has_nick(true), has_user(true),pass(false), last_client(-1)
{
}

Client::~Client()
{
}

void	Client::set_username(const std::string& user)
{
	this->username = user;
	this->has_user = true;
}

void	Client::set_nickname(const std::string& nick)
{
	this->nickname = nick;
	this->has_nick = true;
}


void	Client::set_authenticated(bool auth)
{
	this->authenticated = auth;
}

