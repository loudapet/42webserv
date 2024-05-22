/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/07 10:01:22 by aulicna           #+#    #+#             */
/*   Updated: 2024/05/22 14:37:07 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "webserv.hpp"

bool runWebserv = true;

void	handleSigint(int sigNum)
{
	if (sigNum == SIGINT)
		runWebserv = false;
}

int main(void)
{
	try
	{
		Server	server(PORT_SERVER);

		signal(SIGINT, handleSigint);
		server.start();
		server.stop();
	}
	catch(const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << '\n';
	}
	return (0);
}

//int	main(int argc, char **argv)
//{
//	(void) argv;
//
//	if (argc == 1 || argc == 2)
//	{
//		try
//		{
//			std::string	configFile;
//			if (argc == 1)
//				configFile = "config_files/default.config";
//			else
//				configFile = argv[1];
//			ServerConfig serverConfig();
//		}
//		catch(const std::exception& e)
//		{
//			std::cerr << e.what() << '\n';
//		}
//	}
//	else
//	{
//		std::cerr << "Error: Wrong number of program arguments.\n Usage: "
//			<< "./webserv (default config) or ./webserv [configuration file]"
//			<< std::endl;
//	}
//	return (0);
//}