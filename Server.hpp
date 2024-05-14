/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/10 12:01:17 by aulicna           #+#    #+#             */
/*   Updated: 2024/05/14 19:11:40 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

# include "webserv.hpp"

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

		int					_port;
		int					_serverSocket;
		std::vector<int>	_clientSockets;
		int					_fdMax; // maximum fd number
};

#endif
