/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/22 18:28:00 by aulicna           #+#    #+#             */
/*   Updated: 2024/05/28 15:32:52 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include "webserv.hpp"

class ServerConfig
{
	public:
		ServerConfig(std::string configFile);
		ServerConfig(const ServerConfig& copy);
		ServerConfig	&operator=(const ServerConfig &src);
		~ServerConfig(void);

		std::string	getFileContent(void) const;

		void	removeCommentsAndEmptyLines(void);
		void	detectServerBlocks(void);

		void	printServerBlocks(void) const;
	
	private:
		ServerConfig(void);
		std::string					_configContent;
		std::vector<std::string>	_serverBlocks;

};

#endif
