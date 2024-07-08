/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_cgi1.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okraus <okraus@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/08 16:36:47 by okraus            #+#    #+#             */
/*   Updated: 2024/07/08 16:36:48 by okraus           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// https://computer.howstuffworks.com/cgi3.htm

# include <iostream>

int	main(int argc, char *argv[], char *envp[])
{
	(void)argc;
	(void)argv;
	(void)envp;
	std::cout << "Content-type:text/html\r\n\r\n";
	std::cout << "<html>\n";
	std::cout << "<head>\n";
	std::cout << "<title>Some title</title>\n";
	std::cout << "</head>\n";
	std::cout << "<body>\n";
	std::cout << "<h2> First CGI program </h2>\n";
	std::cout << "</body>\n";
	std::cout << "</html>\n";
	return (0);
} 