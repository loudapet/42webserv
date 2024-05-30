/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/29 12:19:56 by aulicna           #+#    #+#             */
/*   Updated: 2024/05/30 20:05:22 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "ServerConfig.hpp"

ServerConfig::ServerConfig(void)
{
	return ;
}

std::vector<std::string>	splitServerBlock(std::string &serverBlock)
{
	std::vector<std::string>	serverBlockElements;
	size_t						start;
	size_t						end;

	start = 0;
	end = serverBlock.find_first_of("\n ");
	while (end != std::string::npos)
	{
		serverBlockElements.push_back(serverBlock.substr(start, end - start));
		start = end + 1;
		end = serverBlock.find_first_of("\n ", start);
	}
	return (serverBlockElements);
}

bool	validateElement(std::string &element)
{
	size_t	pos;

	pos = element.rfind(";");
	if (pos != element.size() - 1)
		throw(std::runtime_error("Config parser: Invalid ending of an element in a server block."));
	element.erase(pos);
	return (true);
}


ServerConfig::ServerConfig(std::string &serverBlock)
{
	std::vector<std::string>	serverBlockElements;
	bool						inLocationBlock;
	std::istringstream			iss; // convert listen port to unsigned short
	struct sockaddr_in			sa; // validate IP address
	int							fileCheck; // validate root
	struct stat					fileCheckBuff; // validate root
	char						currentDirectory[4096]; // try to find a valid root
	std::string					tmpRoot; // try to find a valid root
	std::vector<std::string>		tmpErrorPages; // to validate error page lines
	


	initServerConfig();
	inLocationBlock = false;
	serverBlockElements = splitServerBlock(serverBlock);
//	std::cout << "elements: " << std::endl;	
//	for (size_t i = 0; i < serverBlockElements.size(); i++)
//		std::cout << serverBlockElements[i] << std::endl;
	// add check for minimal number of elements for the config to be valid
	for (size_t i = 0; i < serverBlockElements.size(); i++)
	{
		if (!inLocationBlock)
		{
			if (serverBlockElements[i] == "listen" && i + 1 < serverBlockElements.size()
				&& validateElement(serverBlockElements[i + 1]))
			{
				if (this->_port != 0)
					throw (std::runtime_error("Config parser: Duplicate listen directive."));
				for (int j = 0; serverBlockElements[i + 1][j]; j++)
				{
					if (!std::isdigit(serverBlockElements[i + 1][j]))
						throw (std::runtime_error("Config parser: Invalid port."));
				}
				iss.str(serverBlockElements[i + 1]);
				if (!(iss >> this->_port) || !iss.eof())
					throw(std::runtime_error("Config parser: Number is out of range for unsigned short."));
			}
			else if (serverBlockElements[i] == "server_name" && i + 1 < serverBlockElements.size()
				&& validateElement(serverBlockElements[i + 1]))
			{
				if (!this->_serverName.empty())
					throw (std::runtime_error("Config parser: Duplicate server_name directive."));
				this->_serverName = serverBlockElements[i + 1];
			}
			else if (serverBlockElements[i] == "host" && i + 1 < serverBlockElements.size()
				&& validateElement(serverBlockElements[i + 1]))
			{
				if (this->_host != 0)
					throw (std::runtime_error("Config parser: Duplicate host directive."));
				if (serverBlockElements[i + 1] == "localhost")
					serverBlockElements[i + 1] = "127.0.0.1";
				if (inet_pton(AF_INET, serverBlockElements[i + 1].c_str(), &(sa.sin_addr)) == 0)
					throw(std::runtime_error("Config parser: Invalid host address."));
				this->_host = inet_addr(serverBlockElements[i + 1].data());
			}
			else if (serverBlockElements[i] == "root" && i + 1 < serverBlockElements.size()
				&& validateElement(serverBlockElements[i + 1]))
			{
				if (!this->_root.empty())
					throw (std::runtime_error("Config parser: Duplicate root directive."));
				// validate or find a valid root
				fileCheck = stat(serverBlockElements[i + 1].c_str(), &fileCheckBuff);
				if (fileCheck != 0)
					throw(std::runtime_error("Config parser: Cannot access root path."));
				if (!(fileCheckBuff.st_mode & S_IFDIR) || fileCheckBuff.st_mode & S_IFREG) // not a directory, but a file
				{
					getcwd(currentDirectory, sizeof(currentDirectory));
					tmpRoot = currentDirectory + serverBlockElements[i + 1];
					fileCheck = stat(serverBlockElements[i + 1].c_str(), &fileCheckBuff);
					if (!(fileCheckBuff.st_mode & S_IFDIR)) // is not a directory
						throw(std::runtime_error("Config parser: Failed to find a valid root."));
					this->_root = tmpRoot;
				}
				else
					this->_root = serverBlockElements[i + 1];
			}
			else if (serverBlockElements[i] == "index" && i + 1 < serverBlockElements.size()
				&& validateElement(serverBlockElements[i + 1]))
			{
				if (!this->_index.empty())
					throw (std::runtime_error("Config parser: Duplicate index directive."));
				this->_index = serverBlockElements[i + 1];
			}
			else if (serverBlockElements[i] == "error_page" && i + 1 < serverBlockElements.size()
				&& validateElement(serverBlockElements[i + 2]))
			{
				for (size_t j = 0; serverBlockElements[i + j].find(';') == std::string::npos && i + j < serverBlockElements.size(); j += 3)
					//tmpErrorPages.push_back(serverBlockElements[i + j]);
					this->_errorPages[atoi(serverBlockElements[i + j + 1].c_str())] = serverBlockElements[i + j + 2];
			}
			else if (serverBlockElements[i] == "location" && i + 1 < serverBlockElements.size()) // validate start and end of the block different to the above
			{
				inLocationBlock = true;
			}
		}
	}
	std::cout << *this << std::endl;
	// check duplications of server_names
}

ServerConfig::ServerConfig(const ServerConfig& copy)
{
	(void)copy;
}

ServerConfig	&ServerConfig::operator = (const ServerConfig &src)
{
	(void)src;
	return (*this);
}

ServerConfig::~ServerConfig(void)
{
	return ;
}

unsigned short	ServerConfig::getPort(void) const
{
	return (this->_port);
}

std::string	ServerConfig::getServerName(void) const
{
	return (this->_serverName);
}

in_addr_t	ServerConfig::getHost(void) const
{
	return (this->_host);
}

std::string	ServerConfig::getRoot(void) const
{
	return (this->_root);
}

std::string	ServerConfig::getIndex(void) const
{
	return (this->_index);
}
	
std::map<short, std::string>	ServerConfig::getErrorPages(void) const
{
	return (this->_errorPages);
}

void	ServerConfig::initServerConfig(void)
{
	this->_port = 0;
	this->_serverName = "";
	this->_host = 0;
	this->_root = "";
	this->_errorPages = std::map<short, std::string>();
}

std::ostream &operator << (std::ostream &o, ServerConfig const &instance)
{
	unsigned int					host;
	std::map<short, std::string>	errorPages;
	
	host = instance.getHost();
	errorPages = instance.getErrorPages();
	o << "___Server Config___" << '\n'
		<< "listen (port): " << instance.getPort() << '\n'
		<< "server_name: " << instance.getServerName() << '\n'
		<< "host: " << host % 256 << "." << host / 256 % 256 << "."	<< host / 65536 % 256 << "." << host / 16777216 << '\n'
		<< "root: " << instance.getRoot() << '\n'
		<< "index: " << instance.getIndex() << '\n'
		<< "error pages: \n";

	for (std::map<short, std::string>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it)
		o << "  " << it->first << ": " << it->second << '\n';
	return (o);
}