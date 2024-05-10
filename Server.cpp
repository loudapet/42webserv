/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/10 11:38:03 by aulicna           #+#    #+#             */
/*   Updated: 2024/05/10 16:58:39 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(int port) : _port(port), _serverSocket(-1)
{
	return ;
}

Server::~Server(void)
{
	this->stop();
	return ;
}

void Server::start(void)
{
	this->_serverSocket = createSocket();
	bindSocket(this->_serverSocket, this->_port);
	listenForConnections(this->_serverSocket);
	std::cout << "Server started on port " << _port << std::endl;
}

void Server::stop(void)
{
	int	tmpClientSocket;

	for (size_t i = 0; i < this->_clientSockets.size(); i++)
	{
		tmpClientSocket = this->_clientSockets[i];
		close(tmpClientSocket);
	}
    this->_clientSockets.clear();
    close(this->_serverSocket);
}

int Server::createSocket(void)
{
	int fdSocket;

	fdSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (fdSocket == -1)
		throw(std::runtime_error("Socket creation failed."));
	return (fdSocket);
}

void Server::bindSocket(int fdSocket, int port)
{
	struct sockaddr_in	serverAddr;
	int					yes = 1;

	if (setsockopt(fdSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
		throw (std::runtime_error("Setsockopt failed."));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);
	if (bind(fdSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
		throw (std::runtime_error("Socket binding failed."));
}

/**
 * inet_ntop(AF_INET, &clientAddr, buf, INET_ADDRSTRLEN) is a call
 * to the inet_ntop function, which converts a network address structure
 * to a string.
*/
void	Server::listenForConnections(int fdSocket)
{
	char	buf[INET_ADDRSTRLEN];
	
	if (listen(fdSocket, SOMAXCONN) == -1)
		throw(std::runtime_error("Socket listening failed."));
	std::cout << "Server listening on port " << this->_port << std::endl;
	while(42)
	{
		struct sockaddr_in	clientAddr;
		socklen_t			lenClientAddr;
		int					clientSocket;

		lenClientAddr = sizeof(clientAddr);
		clientSocket = accept(fdSocket, (struct sockaddr *)&clientAddr, &lenClientAddr);
		if (clientSocket == -1)
			throw(std::runtime_error("Accepting connection failed."));
		std::cout << "New connection accepted from "
			<< inet_ntop(AF_INET, &clientAddr, buf, INET_ADDRSTRLEN)
			<< ". Assigned socket " << clientSocket << '.' << std::endl;
		this->_clientSockets.push_back(clientSocket);
	}
}
