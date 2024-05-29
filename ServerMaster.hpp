/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerMaster.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/29 12:17:04 by aulicna           #+#    #+#             */
/*   Updated: 2024/05/29 13:38:29 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERMASTER_HPP
# define SERVERMASTER_HPP

# include "webserv.hpp"
# include "ServerConfig.hpp"

class ServerMaster
{
	public:
		ServerMaster(std::string configFile);
		~ServerMaster(void);

		std::string	getFileContent(void) const;

		void	removeCommentsAndEmptyLines(void);
		void	detectServerBlocks(void);

		void	printServerBlocks(void) const;
	
	private:
		ServerMaster(void);
		std::string					_configContent;
		std::vector<std::string>	_serverBlocks;
		std::vector<ServerConfig>	_serverConfigs;

};

#endif
