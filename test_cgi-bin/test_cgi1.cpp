/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_cgi1.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okraus <okraus@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/08 16:36:47 by okraus            #+#    #+#             */
/*   Updated: 2024/07/18 13:47:03 by okraus           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// https://computer.howstuffworks.com/cgi3.htm

# include <iostream>
# include <unistd.h>

# define ERROR_BLINKING_COLOUR "\033[1;5;38:5:226;48:5:196m"
# define ERROR_COLOUR "\033[1;38:5:226;48:5:196m"
# define REDBG_COLOUR "\033[1;38:5:231;48:5:196m"
# define GREENBG_COLOUR "\033[1;38:5:0;48:5:46m"
# define BLUEBG_COLOUR "\033[1;38:5:231;48:5:21m"
# define CYANBG_COLOUR "\033[1;38:5:0;48:5:51m"
# define MAGENTABG_COLOUR "\033[1;38:5:231;48:5:201m"
# define YELLOWBG_COLOUR "\033[1;38:5:0;48:5:226m"
# define WHITEBG_COLOUR "\033[1;38:5:0;48:5:231m"
# define BLACKBG_COLOUR "\033[1;38:5:231;48:5:0m"
# define RED_COLOUR "\033[1;38:5:196m"
# define GREEN_COLOUR "\033[1;38:5:46m"
# define BLUE_COLOUR "\033[1;38:5:21m"
# define CYAN_COLOUR "\033[1;38:5:51m"
# define MAGENTA_COLOUR "\033[1;38:5:201m"
# define YELLOW_COLOUR "\033[1;38:5:226m"
# define WHITE_COLOUR "\033[1;38:5:231m"
# define BLACK_COLOUR "\033[1;38:5:0m"
# define NO_COLOUR "\033[0m"

int	main(int argc, char *argv[], char *envp[])
{
	char	buffer[512];
	int		r;
	std::string	str;
	(void)argc;
	// std::cerr << CYANBG_COLOUR "CGI Input" NO_COLOUR << std::endl;
	// std::cerr << CYAN_COLOUR;
	sleep(1);
	while ((r = read(STDIN_FILENO, buffer, 512)) > 0)
	{
		for (int i = 0; i < r; i++)
			std::cerr << buffer[i];
		std::cerr << std::endl;
		std::cerr << "read is: " << r << std::endl;
	}
	// std::cerr << "read is: " << r << std::endl;
	// std::cerr << NO_COLOUR << std::endl;
	// std::cerr << MAGENTABG_COLOUR "CGI ARGS" NO_COLOUR << std::endl;
	// std::cerr << MAGENTA_COLOUR;
	
	std::cout << "Content-type:text/html\r\n\r\n";
	std::cout << "<html>\n";
	std::cout << "<head>\n";
	std::cout << "<title>Some title</title>\n";
	std::cout << "</head>\n";
	std::cout << "<body>\n";
	std::cout << "<h1> First CGI program </h1>\n";
	std::cout << "<h2> Args: </h2>\n";
	std::cout << "<ol>\n";
	for (int i = 0; argv[i]; i++)
	{
		str = argv[i];
		// std::cerr << "argv[" << i << "]:\t" << str << std::endl;
		std::cout << "<li>" << str << "</li>" << std::endl;
	}
	std::cout << "</ol>\n";
	// std::cerr << NO_COLOUR << std::endl;
	// std::cerr << YELLOWBG_COLOUR "CGI ENV" NO_COLOUR << std::endl;
	// std::cerr << YELLOW_COLOUR;
	std::cout << "<h2> Environment: </h2>\n";
	std::cout << "<ol>\n";
	for (int i = 0; envp[i]; i++)
	{
		str = envp[i];
		// std::cerr << "envp[" << i << "]:\t" << str << std::endl; //breaks it somehow?
		std::cout << "<li>" << str << "</li>" << std::endl;
	}
	std::cout << "</ol>\n";
	// std::cerr << NO_COLOUR << std::endl;

	std::cout << "</body>\n";
	std::cout << "</html>\n" << std::endl;
	return (0);
} 