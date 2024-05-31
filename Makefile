# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/02/16 10:28:25 by plouda            #+#    #+#              #
#    Updated: 2024/05/31 13:27:29 by plouda           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME =	parser
SRCS =	parser.cpp HttpHeader.cpp

BUILD_DIR = ./obj
OBJS = $(addprefix $(BUILD_DIR)/, $(SRCS))
OBJS := $(OBJS:%.cpp=%.o)
SRCS := $(addprefix $(SRCS_DIR)/, $(SRCS))

COMP = c++ -g -Wall -Wextra -Werror -std=c++98
SANITIZER := $(if $(shell test -f /usr/local/lib/liblsan.dylib),,-llsan)

GREEN		=	$(shell printf "\033[1;32m")
YELLOW		=	$(shell printf "\033[1;33m")
CYAN		=	$(shell printf "\033[1;36m")
RESET		=	$(shell printf "\033[0m")

all: $(NAME)

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

obj/%.o: src/%.cpp | objdir
	@$(COMP) -c $< -o $@

objdir:
	@mkdir -p obj

clean:
	@rm -rf ./obj
	@echo "$(YELLOW)Object files successfully removed.$(RESET)"

fclean: clean
	@rm -f $(NAME)
	@echo "$(YELLOW)Executable successfully removed.$(RESET)"

re: fclean all

.PHONY: all, clean, fclean, re, bonus