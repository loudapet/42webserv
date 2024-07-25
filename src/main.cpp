/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/07 10:01:22 by aulicna           #+#    #+#             */
/*   Updated: 2024/07/22 15:35:11 by plouda           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ServerMaster.hpp"
#include "../inc/webserv.hpp"

bool g_runWebserv = true;

static void	handleSigint(int sigNum)
{
	(void) sigNum;
	g_runWebserv = false;
}

int	main(int argc, char **argv)
{
	std::string		configFile;
	ServerMaster	serverMaster;

	(void) argv;
	
	signal(SIGINT, handleSigint);
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
			//std::cerr << "Error: " << e.what() << '\n';
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
