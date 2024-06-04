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

void	ServerConfig::validateErrorPagesLine(std::vector<std::string> &errorPageLine)
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


bool	isValidWildcardName(std::vector<std::string> &serverNames)
{
	for (size_t i = 0; i < serverNames.size(); i++)
	{
		if (serverNames[i].find("*") != std::string::npos)
		{
			if (serverNames[i][0] != '*' && serverNames[i][serverNames[i].length() - 1] != '*')
				return (false);
			else
				if (serverNames[i][1] != '.' && serverNames[i][serverNames[i].length() - 2] != '.')
					return (false);
		}
	}
	return (true);
}

ServerConfig::ServerConfig(std::string &serverBlock)
{
	std::vector<std::string>		serverBlockElements;
	bool							inLocationBlock;
	struct sockaddr_in				sa; // validate IP address
	std::vector<std::string>		errorPageLine; // to validate error page lines
//	short						tmpErrorCode; // to validate error page lines
	std::map<short, std::string>	tmpErrorPages;
	bool							rbslInConfig;
	bool							autoindexInConfig;
	
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
			if (serverBlockElements[i] == "listen" && (i + 2) < serverBlockElements.size()
				&& serverBlockElements[i + 1].find_first_of(";") == std::string::npos && validateElement(serverBlockElements[i + 2]))
			{
				this->_port = validateListen(this->_port, serverBlockElements[i + 1]);
				if (serverBlockElements[i + 2] != "default_server" && serverBlockElements[i + 2] != "default")
					throw(std::runtime_error("Config parser: Invalid listen directive."));	
				this->_isDefault = true;
				i += 2;
			}
			else if (serverBlockElements[i] == "listen" && (i + 1) < serverBlockElements.size()
				&& validateElement(serverBlockElements[i + 1]))
			{
				this->_port = validateListen(this->_port, serverBlockElements[i + 1]);
				i++;
			}
			else if (serverBlockElements[i] == "server_name" && (i + 1) < serverBlockElements.size())
			{
				// QUESTION: resolve $hostname?
				if (!this->_serverNames.size() == 0)
					throw (std::runtime_error("Config parser: Duplicate server_name directive."));
				this->_serverNames = extractVectorUntilSemicolon(serverBlockElements, i + 1);
				validateElement(this->_serverNames.back());
				if (!isValidWildcardName(this->_serverNames))
					throw (std::runtime_error("Config parser: Invalid server name."));
				if (this->_serverNames.size() > 0)
					this->_primaryServerName = this->_serverNames[0];
				i += this->_serverNames.size(); // not -1 bcs there is the directive name to skip too
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
				// QUESTION: resolve variables? https://nginx.org/en/docs/varindex.html
				this->_root = validateRoot(this->_root, serverBlockElements[i + 1], "server");
				i++;
			}
			else if (serverBlockElements[i] == "index" && (i + 1) < serverBlockElements.size())
			{
				this->_index = validateIndex(this->_index, serverBlockElements, i + 1, "server");
				i += this->_index.size(); // not -1 bcs there is the directive name to skip too
			}
			else if (serverBlockElements[i] == "client_max_body_size" && (i + 1) < serverBlockElements.size()
				&& validateElement(serverBlockElements[i + 1]))
			{
				// source: https://docs.nginx.com/nginx-management-suite/acm/how-to/policies/request-body-size-limit/
				this->_requestBodySizeLimit = validateRequestBodySizeLimit(rbslInConfig, serverBlockElements[i + 1], "server"); 
				rbslInConfig = true;
				i++;
			}
			else if (serverBlockElements[i] == "autoindex" && (i + 1) < serverBlockElements.size()
				&& validateElement(serverBlockElements[i + 1]))
			{
				this->_autoindex = validateAutoindex(autoindexInConfig, serverBlockElements[i + 1], "server");
				autoindexInConfig = true;
				i++;
			}
			else if (serverBlockElements[i] == "error_page")
			{
				errorPageLine = extractVectorUntilSemicolon(serverBlockElements, i);
				validateErrorPagesLine(errorPageLine);
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
				//std::cout << "location path: " << locationPath << '\n' << "location scope: " << locationScope << std::endl;
				
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
	if (this->_port == 0)
		this->_port = 8000; // If the directive is not present then either *:80 is used if nginx runs with the superuser privileges, or *:8000 otherwise.
	if (this->_root.empty())
		this->_root = "/";
	if (this->_host == 0)
		this->_host = inet_pton(AF_INET, "127.0.0.1", &(sa.sin_addr));
	if (this->_index.size() == 0)
		this->_index.push_back("index.html");
	
	// validate files now that you have the root
	for (size_t i = 0; i < this->_index.size(); i++)
		fileIsValidAndAccessible(this->getRoot() + this->_index[i], "Index file");
	for (std::map<short, std::string>::const_iterator it = this->_errorPages.begin(); it != this->_errorPages.end(); it++)
		fileIsValidAndAccessible(this->getRoot() + it->second, "Error page file");
	// validate mandatory directives
}

ServerConfig::ServerConfig(const ServerConfig& copy)
	: _port(copy._port),
	  _serverNames(copy._serverNames),
	  _host(copy._host),
	  _root(copy._root),
	  _index(copy._index),
	  _errorPages(copy._errorPages),
	  _requestBodySizeLimit(copy._requestBodySizeLimit),
	  _autoindex(copy._autoindex),
	  _locations(copy._locations)
{
}

ServerConfig& ServerConfig::operator=(const ServerConfig& src)
{
	if (this != &src)
	{
		this->_port = src._port;
		this->_serverNames = src._serverNames;
		this->_host = src._host;
		this->_root = src._root;
		this->_index = src._index;
		this->_errorPages = src._errorPages;
		this->_requestBodySizeLimit = src._requestBodySizeLimit;
		this->_autoindex = src._autoindex;
		this->_locations = src._locations;
	}
	return *this;
}

ServerConfig::~ServerConfig(void)
{
	return ;
}

unsigned short	ServerConfig::getPort(void) const
{
	return (this->_port);
}

bool	ServerConfig::getIsDefault(void) const
{
	return (this->_isDefault);
}

const std::string	&ServerConfig::getPrimaryServerName(void) const
{
	return (this->_primaryServerName);
}

const std::vector<std::string>	&ServerConfig::getServerNames(void) const
{
	return (this->_serverNames);
}

in_addr_t	ServerConfig::getHost(void) const
{
	return (this->_host);
}

const std::string	&ServerConfig::getRoot(void) const
{
	return (this->_root);
}

const std::vector<std::string>	&ServerConfig::getIndex(void) const
{
	return (this->_index);
}
	
const std::map<short, std::string>	&ServerConfig::getErrorPages(void) const
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

const std::vector<Location>	&ServerConfig::getLocations(void) const
{
	return (this->_locations);
}

void	ServerConfig::initServerConfig(void)
{
	this->_port = 0;
	this->_isDefault = false;
	this->_serverNames = std::vector<std::string>();
	this->_primaryServerName = "";
	this->_host = 0;
	this->_root = "";
	this->_index = std::vector<std::string>();
	this->_errorPages = std::map<short, std::string>();
	this->_requestBodySizeLimit = REQUEST_BODY_SIZE_LIMIT;
	this->_autoindex = false;
	this->_locations = std::vector<Location>();
}

std::ostream &operator << (std::ostream &o, ServerConfig const &instance)
{
	unsigned int					host;
	std::map<short, std::string>	errorPages;
	std::vector<Location>			locations;
	
	host = instance.getHost();
	errorPages = instance.getErrorPages();
	locations = instance.getLocations();
	o << "___Server Config___" << '\n' << "listen (port): " << instance.getPort();
	if (instance.getIsDefault())
		o << " default_server";
	o << '\n'
		<<  "primary server name: " << instance.getPrimaryServerName() << '\n'
		<< "server names: " << instance.getServerNames() << '\n'
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