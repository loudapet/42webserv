/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerMaster.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/29 12:16:57 by aulicna           #+#    #+#             */
/*   Updated: 2024/06/07 18:03:17 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerMaster.hpp"

ServerMaster::ServerMaster(void)
{
	return ;
}

ServerMaster::ServerMaster(std::string configFile)
{
	std::ifstream	file;
	char			c;
	std::stringstream	tmpFileContent;
	std::set<int> ports;
	int	port;

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

//	// Launch servers
//	for (size_t i = 0; i < this->_serverConfigs.size(); i++)
//		this->_serverConfigs[i].startServer();
		
}

ServerMaster::~ServerMaster(void)
{
	return ;
}

std::string	ServerMaster::getFileContent(void) const
{
	return (this->_configContent);
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
			else if (nested == 0)
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
	if (serverEnd == std::string::npos)
		throw(std::runtime_error("Config parser: Server block has no scope."));
	while (serverStart < this->_configContent.length() - 1 && serverEnd != serverStart)
	{
		if (serverStart == serverEnd)
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

//void	ServerMaster::listenForConnections(void)
//{
//	fd_set			readFds; // temp fds list for select()
//	struct timeval	selectTimer;
//	
//	
////	if (listen(fdSocket, SOMAXCONN) == -1)
////		throw(std::runtime_error("Socket listening failed."));
////	std::cout << "Server listening on port " << this->_port << std::endl;
////	FD_SET(fdSocket, &this->_master); // add the listener to the master set
//	this->_fdMax = 0; // keep track of the biggest fd which so far is the only one we have
//	// main listening loop
//	while(runWebserv)
//	{
//		selectTimer.tv_sec = 1;
//		selectTimer.tv_usec = 0; // could be causing select to fail (with errno of invalid argument) if not set
//		readFds = this->_master; // copy whole fds master list in the fds list for select (only listener socket in the first run)
//		if (select(this->_fdMax + 1, &readFds, NULL, NULL, &selectTimer) == -1)
//			throw(std::runtime_error("Select failed."));
//
//		// run through the existing connections looking for data to read
//		for (int i = 0; i <= this->_fdMax; i++)
//		{
//			if (FD_ISSET(i, &readFds)) // finds a socket with data to read
//			{
//				if (FD_ISSET(i, &readFds) && )
//				if (i == fdSocket) // indicates that the server socket is ready to read which means that a client is attempting to connect
//					acceptConnection();
//				else
//				{
//					handleDataFromClient(i);
//					if (this->_clients.find(i)->second.findValidHeaderEnd())
//					{
//						std::cout << "This will be sent to parser: ";
//						this->_clients.find(i)->second.printDataToParse();
//						this->_clients.find(i)->second.clearDataToParse();
//						std::cout << "This is what stays in the buffer: ";
//						this->_clients.find(i)->second.printReceivedData();
//					}
//				}
//			}
//		}
//		checkForTimeout();
//	}
//}
//
//void	ServerMaster::checkForTimeout(void)
//{
//	for (std::map<int, Client>::iterator it = this->_clients.begin(); it != this->_clients.end(); it++)
//	{
//		if (time(NULL) - it->second.getTimeLastMessage() > CONNECTION_TIMEOUT)
//		{
//			std::cout << "Client " << it->second.getClientSocket() << " timeout. Closing connection now." << std::endl;
//			closeConnection(it->first);
//			return ;
//		}
//	}
//}
