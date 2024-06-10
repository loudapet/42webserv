/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerMaster.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/29 12:16:57 by aulicna           #+#    #+#             */
/*   Updated: 2024/06/10 17:43:07 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ServerMaster.hpp"

ServerMaster::ServerMaster(void)
{
	return ;
}

ServerMaster::ServerMaster(std::string configFile)
{
	std::ifstream		file;
	char				c;
	std::stringstream	tmpFileContent;
	std::set<int> 		ports;
	int					port;

	initServerMaster();
	if (configFile.size() < 5 || configFile.substr(configFile.size() - 5) != ".conf")
		throw(std::runtime_error("Provided config file '" + configFile + "' doesn't have a .conf extension."));
	fileIsValidAndAccessible(configFile, "Config file");
	file.open(configFile.c_str());
	if (!(file >> c)) // check if the file is empty by trying to read a character from it
		throw(std::runtime_error("Provided config file '" + configFile + "' is empty."));
	file.putback(c); //	putting the character back bcs it will be read again later
	tmpFileContent << file.rdbuf();
	file.close();
	this->_configContent = tmpFileContent.str();

//	std::cout << "CONTENT (initial)" << std::endl;
//	std::cout << this->_configContent << std::endl;
	removeCommentsAndEmptyLines();
	
	detectServerBlocks();
//	std::cout << "DETECTED SERVER BLOCKS" << std::endl;
//	printServerBlocks();
	for (size_t i = 0; i < this->_serverBlocks.size(); i++)
	{
		ServerConfig newServer(this->_serverBlocks[i]);
		this->_serverConfigs.push_back(newServer);
	}
	// MORE VALIDATION OF CONFIG - across different configs
	// check duplicates of ports
	for (size_t i = 0; i < this->_serverConfigs.size(); i++)
	{
		port = this->_serverConfigs[i].getPort();
		if (!ports.insert(port).second)
			throw(std::runtime_error("Config parser: Duplicate port detected."));
	}
	// QUESTION: check duplicates of server_names?

	// Launch servers
	for (size_t i = 0; i < this->_serverConfigs.size(); i++)
		this->_serverConfigs[i].startServer();
	prepareServersToListen();
	listenForConnections();
}

ServerMaster::~ServerMaster(void)
{
	std::cout << std::endl;
	std::map<int, Client>::iterator it = this->_clients.begin();
	while (it != this->_clients.end())
	{
		closeConnection(it->first);
		it = this->_clients.begin();
	}
	std::map<int, ServerConfig>::iterator it2 = this->_servers.begin();
	while (it2 != this->_servers.end())
	{
		close(it2->first);
		it2 = this->_servers.begin();
	}
}

std::string	ServerMaster::getFileContent(void) const
{
	return (this->_configContent);
}

void	ServerMaster::initServerMaster(void)
{
	this->_fdMax = -1;
	FD_ZERO(&this->_readFds);
	FD_ZERO(&this->_writeFds);
}

void	ServerMaster::removeCommentsAndEmptyLines(void)
{
	size_t				start;
	size_t				end;
	std::string			line;
	std::stringstream	ss;
	std::string			newFileContent;

	start = this->_configContent.find('#');
	while (start != std::string::npos)
	{
		end = this->_configContent.find('\n', start);
		this->_configContent.erase(start, end - start);
		start = this->_configContent.find('#');
	}
//	std::cout << "CONTENT (removed comments)" << std::endl;
//	std::cout << this->_configContent << std::endl;
	ss.str(this->_configContent);
	while (std::getline(ss, line))
	{
		start = line.find_first_not_of(WHITESPACES);
		end = line.find_last_not_of(WHITESPACES);
		if (start != std::string::npos && end != std::string::npos) // trim whitespaces from the end of the line
			line = line.substr(start, end - start + 1);
		else
			line = "";
		if (!line.empty()) // if line not empty (after removing the start and end whitespaces) add to newFileContent
			newFileContent += line + '\n';
	}
	this->_configContent = newFileContent;
//	std::cout << "CONTENT (removed empty lines)" << std::endl;
//	std::cout << this->_configContent << std::endl;
}


size_t	validateServerBlockStart(size_t pos, std::string &configContent)
{
	size_t	i;

	i = configContent.find_first_not_of(" \t\n\r", pos);
	if (i == std::string::npos)
		return (pos);
	if (configContent.substr(i, 6) != "server")
		throw(std::runtime_error("Config parser: Invalid server scope."));
	i += 6;
	i = configContent.find_first_not_of(" \t\n\r", i);
	if (i == std::string::npos || configContent[i] != '{')
		throw(std::runtime_error("Config parser: Invalid server scope."));
	return (i);
}

size_t	validateServerBlockEnd(size_t pos, std::string &configContent)
{
	size_t	i;
	size_t	nested;

	nested = 0;
	i = pos;
	while (i != std::string::npos)
	{
		i = configContent.find_first_of("{}", i);
		if (i != std::string::npos)
		{
			if (configContent[i] == '{')
				nested++;
			else if (!nested)
				return (i);
			else
				nested--;
			i++;
		}
	}
	return (pos);
}

void	ServerMaster::detectServerBlocks(void)
{
	size_t		serverStart;
	size_t		serverEnd;
	std::string	serverBlock;

	if (this->_configContent.find("server") == std::string::npos)
		throw(std::runtime_error("Config parser: No server block found."));
	serverStart = validateServerBlockStart(0, this->_configContent);
	serverEnd = validateServerBlockEnd(serverStart + 1, this->_configContent);
	if (serverEnd == std::string::npos || serverEnd == serverStart + 1)
		throw(std::runtime_error("Config parser: Server block has no scope."));
	while (serverStart < this->_configContent.length() - 1 && serverEnd != serverStart)
	{
		if (serverEnd == serverStart + 1)
			throw(std::runtime_error("Config parser: Server block has no scope."));
		serverBlock = this->_configContent.substr(serverStart, serverEnd - serverStart + 1);
		this->_serverBlocks.push_back(serverBlock);
		serverStart = validateServerBlockStart(serverEnd + 1, this->_configContent);
		serverEnd = validateServerBlockEnd(serverStart + 1, this->_configContent);
	}
}

void	ServerMaster::printServerBlocks(void) const
{
	for (size_t i = 0; i < this->_serverBlocks.size(); i++)
	{
		std::cout << "Server block " << i << ": " << std::endl;
		std::cout << this->_serverBlocks[i] << std::endl;
	}
}

void	ServerMaster::prepareServersToListen(void)
{
	for(size_t i = 0; i < this->_serverConfigs.size(); i++)
	{
		if (listen(this->_serverConfigs[i].getServerSocket(), SOMAXCONN) == -1)
			throw(std::runtime_error("Socket listening failed."));
		if (fcntl(this->_serverConfigs[i].getServerSocket(), F_SETFL, O_NONBLOCK) == -1)
			throw(std::runtime_error("Fcntl failed."));
		FD_SET(this->_serverConfigs[i].getServerSocket(), &this->_readFds);
		this->_servers.insert(std::make_pair(this->_serverConfigs[i].getServerSocket(), this->_serverConfigs[i]));
		std::cout << "Server '" << this->_serverConfigs[i].getPrimaryServerName() << "' listening on port " << this->_serverConfigs[i].getPort() << "..." << std::endl;
	}
	this->_fdMax = this->_serverConfigs.back().getServerSocket();
}

void	ServerMaster::listenForConnections(void)
{
	fd_set			readFds; // temp fds list for select()
	fd_set			writeFds; // temp fds list for select()
	struct timeval	selectTimer;
	
	
//	FD_SET(fdSocket, &this->_master); // add the listener to the master set
	// main listening loop
	while(42)
	{
		selectTimer.tv_sec = 1;
		selectTimer.tv_usec = 0; // could be causing select to fail (with errno of invalid argument) if not set
		readFds = this->_readFds; // copy whole fds master list in the fds list for select (only listener socket in the first run)
		writeFds = this->_writeFds;
		if (select(this->_fdMax + 1, &readFds, &writeFds, NULL, &selectTimer) == -1)
			throw(std::runtime_error("Select failed. + " + std::string(strerror(errno))));
		// run through the existing connections looking for data to read
		for (int i = 0; i <= this->_fdMax; i++)
		{
			if (FD_ISSET(i, &readFds)) // finds a socket with data to read
			{
				if (FD_ISSET(i, &readFds) && this->_servers.count(i)) // indicates that the server socket is ready to read which means that a client is attempting to connect
					acceptConnection(i);
				else if (FD_ISSET(i, &readFds) && this->_clients.count(i))
				{
					handleDataFromClient(i);
					if (this->_clients.find(i)->second.findValidHeaderEnd())
					{
						/* std::cout << "This will be sent to parser: ";\
						this->_clients.find(i)->second.printDataToParse(); */
						HttpRequest	request;
						request.parseHeader(this->_clients.find(i)->second.getDataToParse());
						// resolve ServerConfig to HttpRequest
						this->_clients.find(i)->second.clearDataToParse(); // clears request line and header fields
						request.validateHeader();
						if (request.readRequestBody(this->_clients.find(i)->second.getReceivedData()) < 0)
							request.readBody = true;
						/* std::cout << "This is what stays in the buffer: ";
						this->_clients.find(i)->second.printReceivedData(); */
					}
				}
				else if (FD_ISSET(i, &writeFds) && this->_clients.count(i))
				{
					// CGI TBA
				}
			}
		}
		checkForTimeout();
	}
}

void ServerMaster::selectServerRules(std::pair<std::string, std::string> parserPair, int clientSocket)
{
    unsigned short      portReceived;
    std::istringstream  iss;
    bool                match;
    struct sockaddr_in  sa;
    in_addr_t           hostReceived;
    in_addr_t           host;
    std::vector<std::string>    serverNames;
	std::map<int, ServerConfig>::const_iterator it;
	int					fdServerConfig;

    // match port
    match = false;
    iss.str(parserPair.second);
    if (!(iss >> portReceived) || !iss.eof())
        throw(std::runtime_error("Server rules: Port number is out of range for unsigned short."));
    for (std::map<int, ServerConfig>::const_iterator it = this->_servers.begin(); it != this->_servers.end(); it++)
    {
        if (it->second.getPort() == portReceived)
        {
            match = true;
            host = it->second.getHost();
            serverNames = it->second.getServerNames();
			fdServerConfig = it->first;
            break ;
        }
    }
    if (!match)
    {
        // Send RESPONSE
        closeConnection(clientSocket);
        return ;
    }
    // detect IP address vs server_name (hostname)
    if (inet_pton(AF_INET, parserPair.first.c_str(), &(sa.sin_addr)) == 0)
        throw(std::runtime_error("Server rules: Invalid host address."));
    else // is host (IP address)
    {
        hostReceived = inet_addr(parserPair.first.data());
        if (hostReceived == host)
        {
			this->_clients.find(clientSocket)->second.setServerConfig(this->_servers.find(fdServerConfig)->second);
            return ;
        }
        else
        {
            // Send RESPONSE
            closeConnection(clientSocket);
            return ;
        }
    }
    // is server_name
    for (size_t i = 0; i < serverNames.size(); i++)
    {
        if (parserPair.first == serverNames[i])
        {
			this->_clients.find(clientSocket)->second.setServerConfig(this->_servers.find(fdServerConfig)->second);
            return ;
        }
    }
    // Send RESPONSE
    closeConnection(clientSocket);
}

/**
 * inet_ntop(AF_INET, &clientAddr, buf, INET_ADDRSTRLEN) is a call
 * to the inet_ntop function, which converts a network address structure
 * to a string.
*/
void	ServerMaster::acceptConnection(int serverSocket)
{
	char		buff[INET_ADDRSTRLEN];
	Client				newClient;
	struct sockaddr_in	clientAddr;
	struct sockaddr_in	serverAddr;
	socklen_t			lenClientAddr;
	int					clientSocket;

	lenClientAddr = sizeof(clientAddr);
	clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &lenClientAddr);
	if (clientSocket == -1)
		throw(std::runtime_error("Accepting connection failed."));
	if (getsockname(clientSocket, (struct sockaddr *)&serverAddr, &lenClientAddr) == -1)
		throw(std::runtime_error("Getsockname failed."));
	newClient.setPortConnectedOn(ntohs(serverAddr.sin_port));
	std::cout << "Client connected to server port: " << newClient.getPortConnectedOn() << std::endl;
	newClient.updateTimeLastMessage();
	addFdToSet(this->_readFds, clientSocket);
	if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) < 0) // so that the sockets don't block each other in nested while on recv
	{
		closeConnection(clientSocket);
		throw(std::runtime_error("Fcntl failed."));
	}
	newClient.setClientSocket(clientSocket);
	this->_clients.insert(std::make_pair(clientSocket, newClient));
	std::cout << "New connection accepted from "
		<< inet_ntop(AF_INET, &clientAddr, buff, INET_ADDRSTRLEN)
		<< ". Assigned socket " << clientSocket << '.' << std::endl;
}

void	ServerMaster::handleDataFromClient(const int clientSocket)
{
	uint8_t							recvBuf[CLIENT_MESSAGE_BUFF]; // Buffer to store received data
	const char						*confirmReceived = "Well received!\n";
	ssize_t							bytesReceived;
	
	memset(recvBuf, 0, sizeof(recvBuf)); // clear the receive buffer
	Client &clientToHandle = this->_clients.find(clientSocket)->second; // reference to the client object
	if ((bytesReceived = recv(clientSocket, recvBuf, sizeof(recvBuf), 0)) <= 0)
	{
		if (bytesReceived == 0) // if the client has closed the connection
			std::cout << "Socket " << clientSocket << " hung up." << std::endl;
		else if (bytesReceived < 0)  // if there was an error receiving data
			std::cerr << "Error receiving data from client!" << std::endl;
		closeConnection(clientSocket);
	}
	else // if data has been received
	{
		clientToHandle.updateTimeLastMessage();
		clientToHandle.updateReceivedData(recvBuf, bytesReceived);
		clientToHandle.trimHeaderEmptyLines();
		std::cout << "Data from client on socket " << clientSocket << ": ";
		clientToHandle.printReceivedData();
		std::cout << std::endl;
		
		// send acknowledgement to the client 
		if (send(clientSocket, confirmReceived, strlen(confirmReceived), 0) == -1)
			std::cerr << "Error sending acknowledgement to client." << std::endl;
	}
}


void	ServerMaster::checkForTimeout(void)
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

void	ServerMaster::addFdToSet(fd_set &set, int fd)
{
	FD_SET(fd, &set);
	if (fd > this->_fdMax) // keep track of the max fd
		this->_fdMax = fd;
}

void	ServerMaster::removeFdFromSet(fd_set &set, int fd)
{
	FD_CLR(fd, &set);
	if (fd == this->_fdMax)
		this->_fdMax -= 1;
}

void	ServerMaster::closeConnection(const int clientSocket)
{
	if (FD_ISSET(clientSocket, &this->_readFds))
		removeFdFromSet(this->_readFds, clientSocket);
	if (FD_ISSET(clientSocket, &this->_writeFds))
		removeFdFromSet(this->_writeFds, clientSocket);
	close(clientSocket); // close the socket	
	this->_clients.erase(clientSocket); // remove from clients map
}
