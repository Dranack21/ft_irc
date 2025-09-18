#include "ft_irc.hpp"

Client::Client() :fd(-1), authenticated(false)
{ 
}

Client::Client(int client_fd) : fd(client_fd), authenticated(false)
{

}
Client::Client(std::string username, std::string nickname) :fd(-1), username(username), nickname(nickname), authenticated(false)
{
}

Client::~Client()
{
}