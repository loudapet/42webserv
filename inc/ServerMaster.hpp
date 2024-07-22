/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerMaster.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okraus <okraus@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/29 12:17:04 by aulicna           #+#    #+#             */
/*   Updated: 2024/07/19 14:07:25 by okraus           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERMASTER_HPP
# define SERVERMASTER_HPP

# include "webserv.hpp"
# include "ServerConfig.hpp"
# include "Client.hpp"
# include "HttpRequest.hpp"
# define MAX_FILE_SIZE 1000000000
# define CGI_BUFFER_SIZE 8092

class ServerMaster
{
	public:
		ServerMaster(void);
		~ServerMaster(void);

		void	runWebserv(const std::string &configFile);
		bool	fdIsSetWrite(int fd) const;
		bool	fdIsSetRead(int fd) const;

	private:
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
