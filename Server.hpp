/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/10 12:01:17 by aulicna           #+#    #+#             */
/*   Updated: 2024/05/16 10:57:29 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

# include "webserv.hpp"
# include "Client.hpp"

class Server
{
	public:
		Server(int port);
		~Server();

		void start(void);
		void stop(void);

	private:
		int		createSocket(void);
		void	bindSocket(int fdSocket, int port);
		void	listenForConnections(int fdSocket);
		void	checkForTimeout(void);
		void	acceptConnection(void);
		void	closeConnection(const int socket);

		int					_port;
		int					_serverSocket;
		std::vector<int>	_clientSockets;
		std::map<int, Client>	_clients;
		int					_fdMax; // maximum fd number
		fd_set				_master; // master fds list
};

#endif
