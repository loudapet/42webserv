/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/10 11:38:03 by aulicna           #+#    #+#             */
/*   Updated: 2024/05/18 17:02:40 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(int port) : _port(port), _serverSocket(-1), _fdMax(-1)
{
	FD_ZERO(&this->_master);
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
	for (std::map<int, Client>::iterator it = this->_clients.begin(); it != this->_clients.end(); it++)
		closeConnection(it->first);
    this->_clients.clear();
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

void	Server::listenForConnections(int fdSocket)
{
	fd_set			readFds; // temp fds list for select()
	struct timeval	selectTimer;
	
	
	if (listen(fdSocket, SOMAXCONN) == -1)
		throw(std::runtime_error("Socket listening failed."));
	std::cout << "Server listening on port " << this->_port << std::endl;
	FD_SET(fdSocket, &this->_master); // add the listener to the master set
	this->_fdMax = fdSocket; // keep track of the biggest fd which so far is the only one we have
	// main listening loop
	while(42)
	{
		selectTimer.tv_sec = 1;
		selectTimer.tv_usec = 0; // could be causing select to fail (with errno of invalid argument) if not set
		readFds = this->_master; // copy whole fds master list in the fds list for select (only listener socket in the first run)
		if (select(this->_fdMax + 1, &readFds, NULL, NULL, &selectTimer) == -1)
			throw(std::runtime_error("Select failed."));

		// run through the existing connections looking for data to read
		for (int i = 0; i <= this->_fdMax; i++)
		{
			if (FD_ISSET(i, &readFds)) // finds a socket with data to read
			{
				if (i == fdSocket) // indicates that the server socket is ready to read which means that a client is attempting to connect
					acceptConnection();
				else
					handleDataFromClient(i);
			}
		}
		checkForTimeout();
	}
}

void	Server::checkForTimeout(void)
{
	for (std::map<int, Client>::iterator it = this->_clients.begin(); it != this->_clients.end(); it++)
	{
		if (time(NULL) - it->second.getTimeLastMessage() > CONNECTION_TIMEOUT)
		{
			std::cout << "Client " << it->second.getClientSocket() << " timeout. Closing connection now." << std::endl;
			closeConnection(it->first);
			return ;
		}
	}
}

/**
 * inet_ntop(AF_INET, &clientAddr, buf, INET_ADDRSTRLEN) is a call
 * to the inet_ntop function, which converts a network address structure
 * to a string.
*/
void	Server::acceptConnection(void)
{
	char		buf[INET_ADDRSTRLEN];
	Client				newClient;
	struct sockaddr_in	clientAddr;
	socklen_t			lenClientAddr;
	int					clientSocket;

	lenClientAddr = sizeof(clientAddr);
	clientSocket = accept(this->_serverSocket, (struct sockaddr *)&clientAddr, &lenClientAddr);
	if (clientSocket == -1)
		throw(std::runtime_error("Accepting connection failed."));
	newClient.updateTimeLastMessage();
	FD_SET(clientSocket, &this->_master); // add to master set
	if (clientSocket > this->_fdMax) // keep track of the max fd
		this->_fdMax = clientSocket;
	// so that the sockets don't block each other in nested while on recv
	if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) < 0)
	{
		closeConnection(clientSocket);
		throw(std::runtime_error(std::string("Fcntl error ") + strerror(errno)));
	}
	newClient.setClientSocket(clientSocket);
	this->_clients.insert(std::make_pair(clientSocket, newClient));
	std::cout << "New connection accepted from "
		<< inet_ntop(AF_INET, &clientAddr, buf, INET_ADDRSTRLEN)
		<< ". Assigned socket " << clientSocket << '.' << std::endl;
}

void	Server::handleDataFromClient(const int clientSocket)
{
	uint8_t							recvBuf[CLIENT_MESSAGE_BUFF]; // Buffer to store received data
	const char						*confirmReceived = "Well received!\n";
	ssize_t							bytesReceived;
	std::map<int, Client>::iterator	it; // Iterator to find the client in the map

	it = this->_clients.find(clientSocket);
	memset(recvBuf, 0, sizeof(recvBuf)); // clear the receive buffer
	if (it == this->_clients.end()) // if the client is not found, return from the function
		return ;
	Client &clientToHandle = it->second; // reference to the client object
	if ((bytesReceived = recv(clientSocket, recvBuf, sizeof(recvBuf), 0)) <= 0)
	{
		if (bytesReceived == 0) // if the client has closed the connection
			std::cout << "Socket " << clientSocket << " hung up." << std::endl;
		else if (bytesReceived < 0)  // if there was an error receiving data
			std::cerr << "Error receiving data from client!" << std::endl;
		closeConnection(clientSocket);
	}
	else // if all data has been received
	{
		clientToHandle.updateTimeLastMessage();
		clientToHandle.updateReceivedData(recvBuf, bytesReceived);
		if (send(clientSocket, confirmReceived, strlen(confirmReceived), 0) == -1)
			std::cerr << "Error sending acknowledgement to client." << std::endl;
		std::cout << "All data received from client on socket " << clientSocket << ": ";
		clientToHandle.printReceivedData();
		clientToHandle.clearReceivedData();
		std::cout << std::endl;
	}
}

void	Server::closeConnection(const int clientSocket)
{
	if (FD_ISSET(clientSocket, &this->_master))
	{
		FD_CLR(clientSocket, &this->_master); // remove from master set
		if (clientSocket == this->_fdMax)
			this->_fdMax -= 1;
	}
	close(clientSocket); // close the socket	
	this->_clients.erase(clientSocket); // remove from clients map
}
