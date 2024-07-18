/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerMaster.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/29 12:17:04 by aulicna           #+#    #+#             */
/*   Updated: 2024/07/18 00:13:52 by aulicna          ###   ########.fr       */
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
		void	acceptConnection(int serverSocket);
		void	handleDataFromClient(const int clientSocket);
		
		void	checkForTimeout(void);
		void	selectServerRules(stringpair_t parserPair, int clientSocket);
		void	closeConnection(const int clientSocket);

		void	addFdToSet(fd_set &set, int fd);
		void	removeFdFromSet(fd_set &set, int fd);
		bool	fdIsSetWrite(int fd) const;
		bool	fdIsSetRead(int fd) const;


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
