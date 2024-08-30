/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/07 10:01:22 by aulicna           #+#    #+#             */
/*   Updated: 2024/08/29 19:51:15 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ServerMaster.hpp"
#include "../inc/webserv.hpp"

bool g_runWebserv = true;

static void	handleSignals(int sigNum)
{
	(void) sigNum;
	g_runWebserv = false;
}

int	main(int argc, char **argv)
{
	std::string		configFile;
	ServerMaster	serverMaster;

	(void) argv;
	
	signal(SIGINT, handleSignals);
	signal(SIGPIPE, handleSignals);
	if (argc == 1 || argc == 2)
	{
		try
		{
			if (argc == 1)
				configFile = "config_files/default.conf";
			else
				configFile = argv[1];
			serverMaster.runWebserv(configFile);
		}
		catch(const std::exception& e)
		{
			Logger::log(ERROR, SERVER, e.what(), "");
		}
	}
	else
	{
		std::cerr << "Error: Wrong number of program arguments.\n Usage: "
			<< "./webserv (default config) or ./webserv [configuration file]"
			<< std::endl;
	}
	return (0);
}
