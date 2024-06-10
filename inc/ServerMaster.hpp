/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerMaster.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/29 12:17:04 by aulicna           #+#    #+#             */
/*   Updated: 2024/06/10 11:20:50 by plouda           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERMASTER_HPP
# define SERVERMASTER_HPP

# include "webserv.hpp"
# include "ServerConfig.hpp"
# include "Client.hpp"
# include "HttpRequest.hpp"


class ServerMaster
{
	public:
		ServerMaster(std::string configFile);
		~ServerMaster(void);

		std::string	getFileContent(void) const;

	private:
		ServerMaster(void);

		void	initServerMaster(void);

		void	removeCommentsAndEmptyLines(void);
		void	detectServerBlocks(void);
		void	printServerBlocks(void) const;

		void	prepareServersToListen(void);
		void	listenForConnections(void);
		void	handleDataFromClient(const int clientSocket);
		void	acceptConnection(int serverSocket);
		
		void	checkForTimeout(void);
		void	addFdToSet(fd_set &set, int fd);
		void	removeFdFromSet(fd_set &set, int fd);
		void	closeConnection(const int clientSocket);

		void	selectServerRules(std::string serverNameReceived, unsigned short portReceived, in_addr_t hostReceived, int clientSocket);

		std::string					_configContent;
		std::vector<std::string>	_serverBlocks;
		std::vector<ServerConfig>	_serverConfigs;
		std::map<int, ServerConfig>	_servers;
		std::map<int, Client>		_clients;

		int						_fdMax; // maximum fd number
		fd_set					_readFds; // master fds list
		fd_set					_writeFds;
};

#endif
