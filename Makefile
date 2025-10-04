### COMPIL #####################################################################

CC      = c++
CFLAGS  = -Wall -Wextra -Werror -std=c++98 -Iincludes
NAME    = ircserv

### SRC ########################################################################

SRCS    = main.cpp \
			srcs/Server.cpp \
			srcs/Client.cpp \
			srcs/Commands.cpp \
			srcs/Channels.cpp\
			srcs/Server_helpers.cpp\
			srcs/Client_helpers.cpp\
			srcs/Commands_helpers.cpp\
			srcs/Channels_helpers.cpp\
			
### OBJS ###########################################################################

OBJDIR  = objs
SRCDIR  = srcs
OBJS    = $(SRCS:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

### COMMANDS #######################################################################

all: $(NAME)

$(NAME): $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all re fclean clean