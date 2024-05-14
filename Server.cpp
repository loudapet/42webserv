/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/10 11:38:03 by aulicna           #+#    #+#             */
/*   Updated: 2024/05/14 18:28:07 by aulicna          ###   ########.fr       */
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
	char		buf[INET_ADDRSTRLEN];
	char		recvbuf[1024];
	const char	*confirmReceived = "Well received!\n";
	fd_set		master; // master fds list
	fd_set		readFds; // temp fds list for select()
	int			fdMax; // maximum fd number
	
	
	if (listen(fdSocket, SOMAXCONN) == -1)
		throw(std::runtime_error("Socket listening failed."));
	std::cout << "Server listening on port " << this->_port << std::endl;
	FD_SET(fdSocket, &master); // add the listener to the master set
	fdMax = fdSocket; // keep track of the biggest fd which so far is the only one we have
	// main listening loop
	while(42)
	{
		readFds = master; // copy whole fds master list in the fds list for select (only listener socket in the first run)
		if (select(fdMax + 1, &readFds, NULL, NULL, NULL) == -1)
			throw(std::runtime_error("Select failed."));

		// run through the existing connections looking for data to read
		for (int i = 0; i <= fdMax; i++)
		{
			if (FD_ISSET(i, &readFds)) // finds a socket with data to read
			{
				if (i == fdSocket)
				{
					// handle new connections
					struct sockaddr_in	clientAddr;
					socklen_t			lenClientAddr;
					int					clientSocket;

					clientSocket = accept(fdSocket, (struct sockaddr *)&clientAddr, &lenClientAddr);
					if (clientSocket == -1)
						throw(std::runtime_error("Accepting connection failed."));
					FD_SET(clientSocket, &master); // add to master set
					if (clientSocket > fdMax) // keep track of the max fd
						fdMax = clientSocket;
					std::cout << "New connection accepted from "
						<< inet_ntop(AF_INET, &clientAddr, buf, INET_ADDRSTRLEN)
						<< ". Assigned socket " << clientSocket << '.' << std::endl;
				}
				else
				{
					// handle data from a client
					ssize_t				bytesReceived;
					
					memset(recvbuf, 0, sizeof(recvbuf));
					if ((bytesReceived = recv(i, recvbuf, sizeof(recvbuf), 0)) <= 0)
					{
						// got error or connection closed by client
						if (bytesReceived == 0)
							std::cout << "Socket " << i << " hung up." << std::endl;
						else if (bytesReceived == -1)
							std::cerr << "Error receiving data from client!!!" << std::endl;
						close(i); // close the socket
						FD_CLR(i, &master); // remove from master set
					}
					else
					{
					//	recvbuf[bytesReceived] = 'a';
						std::cout << "Received from client on socket " << i << ": " << recvbuf;
						// got some data from client
						for (int j = 0; j <= fdMax; j++)
						{
							// send acknowledgement
							if (FD_ISSET(j, &master))
							{
								// not to the listener
								if (j == i)
								{
									if (send(i, confirmReceived, strlen(confirmReceived), 0) == -1)
										std::cerr << "Error sending acknowledgement to client." << std::endl;
								}
							//	// except the listener and ourselves
							//	if (j != fdSocket && j != i)
							//	{
							//		if (send(j, recvbuf, bytesReceived, 0) == -1)
							//			std::cerr << "Error sending." << std::endl;
							//	}
							}
						}
					}
				}
			}		
		}

//		struct sockaddr_in	clientAddr;
//		socklen_t			lenClientAddr;
//		int					clientSocket;
//		ssize_t				bytesReceived;
//
//		lenClientAddr = sizeof(clientAddr);
//		clientSocket = accept(fdSocket, (struct sockaddr *)&clientAddr, &lenClientAddr);
//		if (clientSocket == -1)
//			throw(std::runtime_error("Accepting connection failed."));
//		std::cout << "New connection accepted from "
//			<< inet_ntop(AF_INET, &clientAddr, buf, INET_ADDRSTRLEN)
//			<< ". Assigned socket " << clientSocket << '.' << std::endl;
//
//		// Receive data from client
//		memset(recvbuf, 0, sizeof(recvbuf));
//		bytesReceived = recv(clientSocket, recvbuf, sizeof(recvbuf) - 1, 0);
//		if (bytesReceived == -1)
//			std::cerr << "Error receiving data from client." << std::endl;
//		else
//		{
//			std::cout << "Received from client: " << recvbuf << std::endl;
//			if (send(clientSocket, confirmReceived, strlen(confirmReceived), 0) == -1)
//				std::cerr << "Error sending acknowledgement to client." << std::endl;
//		}
//		this->_clientSockets.push_back(clientSocket);
		
	}
}
