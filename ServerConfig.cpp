/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/29 12:19:56 by aulicna           #+#    #+#             */
/*   Updated: 2024/06/02 16:48:43aulicna          ###   ########.fr       */
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
	end = serverBlock.find_first_not_of(WHITESPACES);
	while (end != std::string::npos)
	{
		start = serverBlock.find_first_not_of(WHITESPACES, start);
		end = serverBlock.find_first_of(WHITESPACES, start);
		if (start != std::string::npos)
		{
			if (end != std::string::npos)
				serverBlockElements.push_back(serverBlock.substr(start, end - start));
			else
				serverBlockElements.push_back(serverBlock.substr(start, serverBlock.length() - start));
		}
		start = end + 1;
	}
	return (serverBlockElements);
}

void	ServerConfig::validateErrorPages(std::vector<std::string> &errorPageLine)
{
	short							tmpErrorCode;
	std::istringstream				iss; // convert error code to short
	std::string						errorPageFileName;
	std::ifstream					errorPageFile;

	if (errorPageLine.size() < 3)
		throw(std::runtime_error("Config parser: Invalid formatting of error_page directive."));
	errorPageFileName = errorPageLine.back();
	if (validateElement(errorPageFileName))
	{
		for (size_t i = 1; i < errorPageLine.size() - 1; i++) // -1 to ignore the page itself
		{
			for (size_t j = 0; j < errorPageLine[i].size(); j++)
			{
				if (!std::isdigit(errorPageLine[i][j]))
					throw (std::runtime_error("Config parser: Invalid error page number."));
			}
			iss.str(errorPageLine[i]);
			if (!(iss >> tmpErrorCode) || !iss.eof())
				throw(std::runtime_error("Config parser: Error page number is out of range for short."));
			iss.str("");
			iss.clear();
			if (tmpErrorCode < 400 || (tmpErrorCode > 426 && tmpErrorCode < 500) || tmpErrorCode > 505)
				throw(std::runtime_error("Config parser: Error page number is out of range of valid error pages."));
			this->_errorPages[tmpErrorCode] = errorPageFileName;
		}
	}
}


ServerConfig::ServerConfig(std::string &serverBlock)
{
	std::vector<std::string>		serverBlockElements;
	bool							inLocationBlock;
	std::istringstream				iss; // convert listen port to unsigned short
	struct sockaddr_in				sa; // validate IP address
	int								fileCheck; // validate root
	struct stat						fileCheckBuff; // validate root
	char							currentDirectory[4096]; // try to find a valid root
	std::string						tmpRoot; // try to find a valid root
	std::vector<std::string>		errorPageLine; // to validate error page lines
//	short						tmpErrorCode; // to validate error page lines
	std::map<short, std::string>	tmpErrorPages;
	bool							rbslInConfig;
	bool							autoindexInConfig;
	char *end;
	long							rbslValue;
	
	initServerConfig();
	inLocationBlock = false;
	rbslInConfig = false;
	autoindexInConfig = false;
	serverBlockElements = splitServerBlock(serverBlock);
	// std::cout << "elements: \n" << serverBlockElements << std::endl;
	// add check for minimal number of elements for the config to be valid
	for (size_t i = 0; i < serverBlockElements.size(); i++)
	{
		if (!inLocationBlock)
		{
			if (serverBlockElements[i] == "listen" && (i + 1) < serverBlockElements.size()
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
					throw(std::runtime_error("Config parser: Port number is out of range for unsigned short."));
				iss.str("");
				iss.clear();
				i++;
			}
			else if (serverBlockElements[i] == "server_name" && (i + 1) < serverBlockElements.size()
				&& validateElement(serverBlockElements[i + 1]))
			{
				if (!this->_serverName.empty())
					throw (std::runtime_error("Config parser: Duplicate server_name directive."));
				this->_serverName = serverBlockElements[i + 1];
				i++;
			}
			else if (serverBlockElements[i] == "host" && (i + 1) < serverBlockElements.size()
				&& validateElement(serverBlockElements[i + 1]))
			{
				if (this->_host != 0)
					throw (std::runtime_error("Config parser: Duplicate host directive."));
				if (serverBlockElements[i + 1] == "localhost")
					serverBlockElements[i + 1] = "127.0.0.1";
				if (inet_pton(AF_INET, serverBlockElements[i + 1].c_str(), &(sa.sin_addr)) == 0)
					throw(std::runtime_error("Config parser: Invalid host address."));
				this->_host = inet_addr(serverBlockElements[i + 1].data());
				i++;
			}
			else if (serverBlockElements[i] == "root" && (i + 1) < serverBlockElements.size()
				&& validateElement(serverBlockElements[i + 1]))
			{
				if (!this->_root.empty())
					throw (std::runtime_error("Config parser: Duplicate root directive."));
				// check the absolute path (raw string from config) and if invalid, check relative path that is interpreted as relative to the current working directory
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
				i++;
			}
			else if (serverBlockElements[i] == "index" && (i + 1) < serverBlockElements.size()
				&& validateElement(serverBlockElements[i + 1]))
			{
				if (!this->_index.empty())
					throw (std::runtime_error("Config parser: Duplicate index directive."));
				this->_index = serverBlockElements[i + 1];
				i++;
			}
			else if (serverBlockElements[i] == "client_max_body_size" && (i + 1) < serverBlockElements.size()
				&& validateElement(serverBlockElements[i + 1]))
			{
				if (rbslInConfig)
					throw (std::runtime_error("Config parser: Duplicate client_max_body_size directive."));
				for (size_t j = 0; j < serverBlockElements[i + 1].size(); j++)
				{
					if (!std::isdigit(serverBlockElements[i + 1][j]))
						throw (std::runtime_error("Config parser: Invalid client_max_body_size number."));
				}
				rbslValue = strtol(serverBlockElements[i + 1].c_str(), &end, 10);
				if (end[0] != 'K' && end[0] != 'M' && end[0] != '\0')
					throw (std::runtime_error("Config parser: Invalid client_max_body_size number."));
				int multiplier = 1;
				if (end[0] == 'K')
				{
					if (rbslValue > INT_MAX / 1024 || rbslValue < 0)
						throw(std::overflow_error("Config parser: client_max_body_size too large."));
					multiplier = 1024;
				}
				else if (end[0] == 'M')
				{
					if (rbslValue > INT_MAX / (1024 * 1024) || rbslValue < 0)
						throw(std::overflow_error("Config parser: client_max_body_size too large."));
					multiplier = 1024 * 1024;
				}
				else if (end[0] == '\0' && rbslValue > INT_MAX)
					throw(std::overflow_error("Config parser: client_max_body_size too large."));
				this->_requestBodySizeLimit = static_cast<int>(rbslValue) * multiplier;
				rbslInConfig = true;
				i++;
			}
			else if (serverBlockElements[i] == "autoindex" && (i + 1) < serverBlockElements.size()
				&& validateElement(serverBlockElements[i + 1]))
			{
				if (autoindexInConfig)
					throw (std::runtime_error("Config parser: Duplicate autoindex directive."));
				if (serverBlockElements[i + 1] != "on" && serverBlockElements[1 + i] != "off")
					throw (std::runtime_error("Config parser: Wrong syntax of autoindex directive."));
				if (serverBlockElements[i + 1] == "on")
					this->_autoindex = true;
				autoindexInConfig = true;
				i++;
			}
			else if (serverBlockElements[i] == "error_page")
			{
				size_t j = 0;
				while (serverBlockElements[i + j].find(';') == std::string::npos && i + j < serverBlockElements.size())
				{
					errorPageLine.push_back(serverBlockElements[i + j]);
					j++;
				}
				errorPageLine.push_back(serverBlockElements[i + j]);
				validateErrorPages(errorPageLine);
				i += errorPageLine.size() - 1;
				errorPageLine.clear();
			}
			else if (serverBlockElements[i] == "location" && (i + 1) < serverBlockElements.size()) // validate start and end of the block different to the above
			{
				size_t	posBracket;
				std::string	locationPath;
				std::vector<std::string> locationScope;

				for (posBracket = i; posBracket < serverBlockElements.size(); posBracket++)
				{
						if (serverBlockElements[posBracket] == "{")
							break ;
				}
				if (posBracket == serverBlockElements.size() - 1)
					throw(std::runtime_error("Invalid location directive scope."));
				if (posBracket != i + 2)
					throw(std::runtime_error("Invalid start of a location directive."));
				locationPath = serverBlockElements[i + 1];
				for (size_t j = 2; i + j < serverBlockElements.size() && serverBlockElements[i + j] != "}"; j++)
					locationScope.push_back(serverBlockElements[i + j]);
				locationScope.push_back("}");
				std::cout << "location path: " << locationPath << '\n'
					<< "location scope: " << locationScope << std::endl;

				Location newLocation(locationPath, locationScope);
				this->_locations.push_back(newLocation);
				
				i += locationScope.size() + 1; // if 'location' and location path from the config would be included, it'd be -1, but those 2 aren't in the vector
				locationScope.clear();
			//	inLocationBlock = true;
			}
			else if (serverBlockElements[i] != "{" && serverBlockElements[i] != "}")
			{
				if (!inLocationBlock)
				{
					std::cout << "unsupported: " << serverBlockElements[i] << std::endl;
					throw (std::runtime_error("Config parser: Invalid directive in server block."));
				}
			}
		}
	}
	std::cout << *this << std::endl;
	// check duplications of server_names

	// set empty values
	if (this->_root.empty())
		this->_root = "/";
	if (this->_host == 0)
		this->_host = inet_pton(AF_INET, "127.0.0.1", &(sa.sin_addr));
	if (this->_index.empty())
		this->_index = "index.html";
	
	// validate files now that you have the root
	if (access((this->getRoot() + this->_index).c_str(), 0) < 0)
		throw(std::runtime_error("Config parser: Index file '" + this->_index + "' is an invalid file."));
	if (access((this->getRoot() + this->_index).c_str(), 4) < 0)
		throw(std::runtime_error("Config parser: Index file '" + this->_index + "' is not accessible."));
	for (std::map<short, std::string>::const_iterator it = this->_errorPages.begin(); it != this->_errorPages.end(); it++)
	{
		if (access((this->getRoot() + it->second).c_str(), 0) < 0)
			throw(std::runtime_error("Config parser: Error page file '" + it->second + "' is an invalid file."));
		if (access((this->getRoot() + it->second).c_str(), 4) < 0)
			throw(std::runtime_error("Config parser: Error page file '" + it->second + "' is not accessible."));
	}
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
	if (this->_root[this->_root.size() - 1] != '/')
		return (this->_root + "/");
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

unsigned int	ServerConfig::getRequestBodySizeLimit(void) const
{
	return (this->_requestBodySizeLimit);
}

bool	ServerConfig::getAutoindex(void) const
{
	return (this->_autoindex);
}

std::vector<Location>	ServerConfig::getLocations(void) const
{
	return (this->_locations);
}

void	ServerConfig::initServerConfig(void)
{
	this->_port = 0;
	this->_serverName = "";
	this->_host = 0;
	this->_root = "";
	this->_errorPages = std::map<short, std::string>();
	this->_requestBodySizeLimit = REQUEST_BODY_SIZE_LIMIT;
}

std::ostream &operator << (std::ostream &o, ServerConfig const &instance)
{
	unsigned int					host;
	std::map<short, std::string>	errorPages;
	std::vector<Location>			locations;
	
	host = instance.getHost();
	errorPages = instance.getErrorPages();
	locations = instance.getLocations();
	o << "___Server Config___" << '\n'
		<< "listen (port): " << instance.getPort() << '\n'
		<< "server_name: " << instance.getServerName() << '\n'
		<< "host: " << host % 256 << "." << host / 256 % 256 << "."	<< host / 65536 % 256 << "." << host / 16777216 << '\n'
		<< "root: " << instance.getRoot() << '\n'
		<< "index: " << instance.getIndex() << '\n'
		<< "error pages: \n";
	for (std::map<short, std::string>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it)
		o << it->first << ": " << it->second << '\n';
	o << "client_max_body_size (requestBodySizeLimit): " << instance.getRequestBodySizeLimit() << '\n'
		<< "autoindex: " << instance.getAutoindex() << '\n';
	for (size_t i = 0; i < locations.size(); i++)
		o << locations[i] << '\n';
	return (o);
}