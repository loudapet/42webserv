/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/07 10:01:22 by aulicna           #+#    #+#             */
/*   Updated: 2024/06/09 16:11:14 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerMaster.hpp"
#include "webserv.hpp"

int	main(int argc, char **argv)
{
	std::string	configFile;

	(void) argv;

	if (argc == 1 || argc == 2)
	{
		try
		{
			if (argc == 1)
				configFile = "config_files/default.conf";
			else
				configFile = argv[1];
			ServerMaster serverMaster(configFile);
		}
		catch(const std::exception& e)
		{
			std::cerr << "Error: " << e.what() << '\n';
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
