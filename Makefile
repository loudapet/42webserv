# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/02/16 10:28:25 by plouda            #+#    #+#              #
#    Updated: 2024/07/22 15:49:28 by plouda           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME =	webserver
SRCS =	HttpRequest.cpp \
		Client.cpp \
		Location.cpp \
		main.cpp \
		ServerConfig.cpp \
		ServerMaster.cpp \
		Utils.cpp \
		HttpResponse.cpp \
		ResponseException.cpp \
		Mime.cpp \
		Logger.cpp

BUILD_DIR = ./obj
OBJS = $(addprefix $(BUILD_DIR)/, $(SRCS))
OBJS := $(OBJS:%.cpp=%.o)
SRCS := $(addprefix $(SRCS_DIR)/, $(SRCS))

SRCS_CLIENT = mainClient.cpp
OBJS_CLIENT = $(SRCS_CLIENT:%.cpp=%.o)

COMP = c++ -g -Wall -Wextra -Werror -std=c++98
SANITIZER := $(if $(shell test -f /usr/local/lib/liblsan.dylib),,-llsan)

GREEN		=	$(shell printf "\033[1;32m")
YELLOW		=	$(shell printf "\033[1;33m")
CYAN		=	$(shell printf "\033[1;36m")
RESET		=	$(shell printf "\033[0m")

all: $(NAME) client #cgi

$(NAME): $(OBJS)
	@echo "$(YELLOW)Initiating compilation of '${NAME}'$(RESET)"
	@echo "$(YELLOW)Linking objects...$(RESET)"
	@$(COMP) $(OBJS) -o ${NAME}
	@echo "$(GREEN)Compilation of '${NAME}' successful.$(RESET)"

san: $(OBJS)
	@echo "$(YELLOW)Initiating compilation of '${NAME}'$(RESET)"
	@echo "$(YELLOW)Linking objects...$(RESET)"
	@$(COMP) $(OBJS) -o ${NAME} ${SANITIZER}
	@echo "$(GREEN)Compilation of '${NAME}' successful.$(RESET)"

client: $(OBJS_CLIENT)
	@echo "$(CYAN)Compiling client..$(RESET)"
	@$(COMP) $(OBJS_CLIENT) -o client
	@echo "$(GREEN)Compilation of 'client' successful.$(RESET)"

# cgi: ./www/html/cgi-bin/test_cgi1.cpp
# 	@echo "$(CYAN)Compiling cgi...$(RESET)"
# 	$(COMP) ./www/html/cgi-bin/test_cgi1.cpp -o ./www/html/cgi-bin/test1.cgi
# 	@echo "$(GREEN)Compilation of 'cgi' complete.$(RESET)"

%.o: %.cpp
	@$(COMP) -c $< -o $@

obj/%.o: src/%.cpp | objdir
	@$(COMP) -c $< -o $@

objdir:
	@mkdir -p obj

clean:
	@rm -rf ./obj
	@rm -f $(OBJS_CLIENT)
	@echo "$(YELLOW)Object files successfully removed.$(RESET)"

fclean: clean
	@rm -f $(NAME)
	@rm -f client
#	rm -f ./www/html/cgi-bin/test1.cgi
	@echo "$(YELLOW)Executable '${NAME}' successfully removed.$(RESET)"
	@echo "$(YELLOW)Executable 'client' successfully removed.$(RESET)"
#	@echo "$(YELLOW)Executable 'test1.cgi' successfully removed.$(RESET)"

re: fclean all

.PHONY: all, clean, fclean, re, bonus