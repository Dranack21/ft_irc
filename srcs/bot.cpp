#include "ft_irc.hpp"

void Server_class::create_bot()
{
    this->clients[-2] = Client(-2);
    this->clients[-2].set_nickname("Bot");
    this->clients[-2].set_username("bot");
    this->clients[-2].set_realname("IRC Bot");
    this->clients[-2].set_authenticated(true);
    this->clients[-2].pass = true;
    
    std::cout << "Bot created: nickname=Bot" << std::endl;
}

void Server_class::handle_bot_message(int client_fd, const std::string& message)
{
    std::string response;
    std::string msg_upper = to_upper(message);
    
    if (msg_upper == "HELP\r\n" || msg_upper == "HELP" || msg_upper.empty())
    {
        response = ":Bot!bot@ft_irc.42.fr PRIVMSG " + this->clients[client_fd].get_nickname() + 
                  " :Available commands: HELP, TIME, HELLO, INFO\r\n";
    }
    else if (msg_upper == "TIME\r\n" || msg_upper == "TIME")
    {
		std::cout << "eeeee" << std::endl;
        std::time_t now = std::time(0);
        char* dt = std::ctime(&now);
        std::string time_str(dt);
        if (!time_str.empty() && time_str[time_str.size() - 1] == '\n')
            time_str.erase(time_str.size() - 1);
        
        response = ":Bot!bot@ft_irc.42.fr PRIVMSG " + this->clients[client_fd].get_nickname() + 
                  " :Current server time: " + time_str + "\r\n";
    }
    else if (msg_upper == "HELLO\r\n" || msg_upper == "HELLO")
    {
        response = ":Bot!bot@ft_irc.42.fr PRIVMSG " + this->clients[client_fd].get_nickname() + 
                  " :Hello " + this->clients[client_fd].get_nickname() + "! Welcome to ft_irc! 👋\r\n";
    }
    else if (msg_upper == "INFO\r\n" || msg_upper == "INFO")
    {
        response = ":Bot!bot@ft_irc.42.fr PRIVMSG " + this->clients[client_fd].get_nickname() + 
                  " :ft_irc server v" + this->server_version + " - Created: " + this->creation_date + "\r\n";
    }
	else if (msg_upper.substr(0, 4) == "PLAY")
		this->play_bot(client_fd, msg_upper);
    else
    {
        response = ":Bot!bot@ft_irc.42.fr PRIVMSG " + this->clients[client_fd].get_nickname() + 
                  " :Unknown command. Type 'help' for available commands.\r\n";
    }
    send(client_fd, response.c_str(), response.length(), 0);
}

void Server_class::play_bot(int client_fd, const std::string &message)
{
	static int game_counter = 0;
    std::string response;
    std::string msg_upper = to_upper(message);
    
    // Extract the move from the message (should be "PLAY R", "PLAY P", or "PLAY S")
    size_t space_pos = msg_upper.find(' ');
    std::string player_move;
    
    if (space_pos != std::string::npos && space_pos + 1 < msg_upper.length())
    {
        player_move = msg_upper.substr(space_pos + 1);
        if (player_move.find("\r\n") != std::string::npos)
            player_move = player_move.substr(0, player_move.find("\r\n"));
    }
    if (player_move != "R" && player_move != "P" && player_move != "S")
    {
        response = ":Bot!bot@ft_irc.42.fr PRIVMSG " + this->clients[client_fd].get_nickname() + 
                  " :Invalid move! Use: PLAY R (rock), PLAY P (paper), or PLAY S (scissors)\r\n";
        send(client_fd, response.c_str(), response.length(), 0);
        return;
    }
    std::srand(std::time(0) + client_fd + game_counter++);
    int bot_choice = std::rand() % 3;
    std::string bot_move;
    std::string bot_move_name;
    
    if (bot_choice == 0)
    {
        bot_move = "R";
        bot_move_name = "Rock";
    }
    else if (bot_choice == 1)
    {
        bot_move = "P";
        bot_move_name = "Paper";
    }
    else
    {
        bot_move = "S";
        bot_move_name = "Scissors";
    }
    std::string player_move_name;
    if (player_move == "R")
        player_move_name = "Rock";
    else if (player_move == "P")
        player_move_name = "Paper";
    else
        player_move_name = "Scissors";
    response = ":Bot!bot@ft_irc.42.fr PRIVMSG " + this->clients[client_fd].get_nickname() + 
              " :You chose " + player_move_name + ", I chose " + bot_move_name + "... ";
    std::string result;
    if (player_move == bot_move)
        result = "It's a tie! 🤝";
    else if ((player_move == "R" && bot_move == "S") ||
             (player_move == "P" && bot_move == "R") ||
             (player_move == "S" && bot_move == "P"))
        result = "You win! 🎉";
    else
        result = "I win! 🤖";
    response += result + "\r\n";
    send(client_fd, response.c_str(), response.length(), 0);
}