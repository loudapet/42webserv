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

#include "../inc/webserv.hpp"
#include "../inc/ServerConfig.hpp"
#include "../inc/Mime.hpp"

ServerConfig::ServerConfig(void)
{
	return ;
}


ServerConfig::ServerConfig(std::string &serverBlock)
{
	std::vector<std::string>		serverBlockElements;
	bool							inLocationBlock;
	struct sockaddr_in				sa; // validate IP address
	std::vector<std::string>		errorPageLine; // to validate error page lines
	std::map<unsigned short, std::string>	tmpErrorPages;
	bool							rbslInConfig;
	bool							autoindexInConfig;
	bool						allowMethodsInConfig;
	bool						returnInConfig;
	std::vector<std::string>	allowMethodsLine;
	std::string 				validMethodsArray[] = {"GET", "POST", "DELETE"};
	std::set<std::string> 		validMethods(validMethodsArray, validMethodsArray + sizeof(validMethodsArray) / sizeof(validMethodsArray[0]));
	
	initServerConfig();
	inLocationBlock = false;
	rbslInConfig = false;
	autoindexInConfig = false;
	allowMethodsInConfig = false;
	returnInConfig = false;
	serverBlockElements = splitBlock(serverBlock);
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
				if (!(this->_serverNames.size() == 0))
					throw (std::runtime_error("Config parser: Duplicate server_name directive."));
				this->_serverNames = extractVectorUntilSemicolon(serverBlockElements, i + 1);
				validateElement(this->_serverNames.back());
				if (this->_serverNames.size() > 0)
					this->_primaryServerName = this->_serverNames[0];
				i += this->_serverNames.size(); // not -1 bcs there is the directive name to skip too
			}
			else if (serverBlockElements[i] == "host" && (i + 1) < serverBlockElements.size()
				&& validateElement(serverBlockElements[i + 1]))
			{
				// QUESTION: detect addresses that will fail sock binding?
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
				this->_autoindex = validateOnOffDirective(autoindexInConfig, "autoindex", serverBlockElements[i + 1], "server");
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
			else if (serverBlockElements[i] == "return" && (i + 2) < serverBlockElements.size()
				&& serverBlockElements[i + 1].find_first_of(";") == std::string::npos && validateElement(serverBlockElements[i + 2]))
			{
				if (returnInConfig)
					throw(std::runtime_error("Config parser: Duplicate return directive."));
				// source: https://nginx.org/en/docs/http/ngx_http_rewrite_module.html#return
				this->_returnCode = validateReturnCode(serverBlockElements[i + 1]);
				this->_returnURLOrBody = serverBlockElements[i + 2];		
				this->_isRedirect = true;
				returnInConfig = true;
				i += 2;						
			}
			else if (serverBlockElements[i] == "return" && (i + 1) < serverBlockElements.size()
				&& validateElement(serverBlockElements[i + 1]))
			{
				if (returnInConfig)
					throw(std::runtime_error("Config parser: Duplicate return directive."));
				this->_returnCode = 302;
				this->_returnURLOrBody = serverBlockElements[i + 1];
				if (this->_returnURLOrBody.substr(0, 7) != "http://" && this->_returnURLOrBody.substr(0, 8) != "https://")
					throw(std::runtime_error("Config parser: Invalid URL in return directive. In this format, the directive is assumed to represent 'return [URL];' and the [URL] needs to start with 'http://' or 'https://'."));
				this->_isRedirect = true;
				returnInConfig = true;
				i++;
			}
			else if (serverBlockElements[i] == "location" && (i + 1) < serverBlockElements.size()) // validate start and end of the block different to the above
			{
				size_t						posBracket;
				std::string					locationPath;
				std::vector<std::string>	locationScope;

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
				Location newLocation(locationPath, locationScope);
				this->_locations.push_back(newLocation);
				i += locationScope.size() + 1; // if 'location' and location path from the config would be included, it'd be -1, but those 2 aren't in the vector
				locationScope.clear();
			}
			else if (serverBlockElements[i] == "allow_methods")
			{
				if (allowMethodsInConfig)
					throw(std::runtime_error("Config parser: Duplicate allow_methods directive in a location block."));
				allowMethodsLine = extractVectorUntilSemicolon(serverBlockElements, i + 1);
				validateElement(allowMethodsLine.back());
				for (size_t i = 0; i < allowMethodsLine.size(); i++)
				{
					if (validMethods.find(allowMethodsLine[i]) == validMethods.end())
						throw(std::runtime_error("Config parser: Invalid method '" + allowMethodsLine[i] + "'."));
					if (!this->_allowMethods.insert(allowMethodsLine[i]).second)
						throw(std::runtime_error("Config parser: Duplicate method '" + allowMethodsLine[i] + "'."));
				}
				i += allowMethodsLine.size(); // not -1 bcs there is the directive to skip too
				allowMethodsLine.clear();
			}
			else if (serverBlockElements[i] == "mime_types" && (i + 1) < serverBlockElements.size()
				&& validateElement(serverBlockElements[i + 1]))
			{
				if (this->_mimeTypesFile != "")
					throw(std::runtime_error("Config parser: Duplicate mime_types directive."));
				if (access(serverBlockElements[i + 1].c_str(), F_OK) < 0)
					throw(std::runtime_error("Config parser: Mime types file at '" + serverBlockElements[i + 1] + "' is an invalid file."));
				this->_mimeTypesFile = serverBlockElements[i + 1];
				this->_mimeTypes.parseMimeTypes(this->_mimeTypesFile);
				i++;
			}
			else if (serverBlockElements[i] != "{" && serverBlockElements[i] != "}")
				if (!inLocationBlock)
					throw (std::runtime_error("Config parser: Invalid directive in a server block."));
		}
	}
	// set empty values
	if (this->_port == 0)
		this->_port = 8002; // If the directive is not present then either *:80 is used if nginx runs with the superuser privileges, or *:8000 otherwise.
	if (this->_root.empty())
		this->_root = "./";
	if (this->_host == 0)
		this->_host = inet_addr(std::string("127.0.0.1").data());
	if (this->_index.size() == 0)
		this->_index.push_back("index.html");
	if (this->_allowMethods.size() == 0)
		this->_allowMethods.insert("GET");
	// validate files now that you have the root
	for (size_t i = 0; i < this->_index.size(); i++)
		fileIsValidAndAccessible(this->getRoot() + this->_index[i], "Index");
	for (std::map<unsigned short, std::string>::const_iterator it = this->_errorPages.begin(); it != this->_errorPages.end(); it++)
		fileIsValidAndAccessible(this->getRoot() + it->second, "Error page");
	completeLocations(); // the location is completed only here as access to the server values is needed
	validateLocations();
}

std::map<std::string, std::vector<std::string> > readMimeTypesFile(const std::string &mimeTypesFile)
{
	std::map< std::string, std::vector<std::string> >	mimeTypesDict;

	(void) mimeTypesFile;
	mimeTypesDict = std::map< std::string, std::vector<std::string> >();
	return (mimeTypesDict);	
}

ServerConfig::ServerConfig(const ServerConfig& copy)
{
	this->_port = copy._port;
	this->_isDefault = copy._isDefault;
	this->_serverNames = copy._serverNames;
	this->_primaryServerName = copy._primaryServerName;
	this->_host = copy._host;
	this->_root = copy._root;
	this->_index = copy._index;
	this->_errorPages = copy._errorPages;
	this->_requestBodySizeLimit = copy._requestBodySizeLimit;
	this->_autoindex = copy._autoindex;
	this->_allowMethods = copy._allowMethods;
	this->_returnURLOrBody = copy._returnURLOrBody;
	this->_returnCode = copy._returnCode;
	this->_isRedirect = copy._isRedirect;
	this->_locations = copy._locations;
	this->_serverSocket = copy._serverSocket;
	this->_serverAddr = copy._serverAddr;
	this->_mimeTypesFile = copy._mimeTypesFile;
	this->_mimeTypes = copy._mimeTypes;
}

ServerConfig& ServerConfig::operator = (const ServerConfig& src)
{
	if (this != &src)
	{
		this->_port = src._port;
		this->_isDefault = src._isDefault;
		this->_serverNames = src._serverNames;
		this->_primaryServerName = src._primaryServerName;
		this->_host = src._host;
		this->_root = src._root;
		this->_index = src._index;
		this->_errorPages = src._errorPages;
		this->_requestBodySizeLimit = src._requestBodySizeLimit;
		this->_autoindex = src._autoindex;
		this->_allowMethods = src._allowMethods;
		this->_returnURLOrBody = src._returnURLOrBody;
		this->_returnCode = src._returnCode;
		this->_isRedirect = src._isRedirect;
		this->_locations = src._locations;
		this->_serverSocket = src._serverSocket;
		this->_serverAddr = src._serverAddr;
		this->_mimeTypesFile = src._mimeTypesFile;
		this->_mimeTypes = src._mimeTypes;
	}
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
	
const std::map<unsigned short, std::string>	&ServerConfig::getErrorPages(void) const
{
	return (this->_errorPages);
}

int	ServerConfig::getRequestBodySizeLimit(void) const
{
	return (this->_requestBodySizeLimit);
}

bool	ServerConfig::getAutoindex(void) const
{
	return (this->_autoindex);
}

const std::set<std::string>			&ServerConfig::getAllowMethods(void) const
{
	return (this->_allowMethods);
}

const std::string		&ServerConfig::getReturnURLOrBody(void) const
{
	return (this->_returnURLOrBody);
}

unsigned short	ServerConfig::getReturnCode(void) const
{
	return (this->_returnCode);
}

bool	ServerConfig::getIsRedirect(void) const
{
	return (this->_isRedirect);
}

const std::vector<Location>	&ServerConfig::getLocations(void) const
{
	return (this->_locations);
}

int	ServerConfig::getServerSocket(void) const
{
	return (this->_serverSocket);
}

const std::string	&ServerConfig::getMimeTypeFile(void) const
{
	return (this->_mimeTypesFile);
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
	this->_errorPages = std::map<unsigned short, std::string>();
	this->_requestBodySizeLimit = REQUEST_BODY_SIZE_LIMIT;
	this->_autoindex = false;
	this->_returnURLOrBody = "";
	this->_returnCode = 0;
	this->_isRedirect = false;
	this->_locations = std::vector<Location>();
	this->_mimeTypesFile = "";
	this->_mimeTypes = Mime();
}

void	ServerConfig::validateErrorPagesLine(std::vector<std::string> &errorPageLine)
{
	unsigned short					errorCode;
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
			if (!(iss >> errorCode) || !iss.eof())
				throw(std::runtime_error("Config parser: Error page number is out of range for valid error codes."));
			iss.str("");
			iss.clear();
			if (errorCode < 400 || (errorCode > 426 && errorCode < 500) || errorCode > 505)
				throw(std::runtime_error("Config parser: Error page number is out of range for valid error pages."));
			this->_errorPages[errorCode] = errorPageFileName;
		}
	}
}

void	ServerConfig::completeLocations(void)
{
	
	for (size_t i = 0; i < this->_locations.size(); i++)
	{
	// add server root if none defined
		if (!this->_locations[i].getIsCgi()) // not CGI
		{
			if (this->_locations[i].getRoot().empty())
				this->_locations[i].setRoot(this->_root);
			if (this->_locations[i].getIndex().empty())
				this->_locations[i].setIndex(this->_index);
			if (this->_locations[i].getRequestBodySizeLimit() == -1)
				this->_locations[i].setRequestBodySizeLimit(this->_requestBodySizeLimit);
			// what if difference between server and location scope directives - e.g. autoindex off in server but off in this->_locations[i]
			if (this->_locations[i].getAutoindex() == -1)
				this->_locations[i].setAutoindex(this->_autoindex);
		}
		for (std::map<unsigned short, std::string>::const_iterator it = this->_errorPages.begin(); it != this->_errorPages.end(); it++)
		{
			if (this->_locations[i].getErrorPages().find(it->first) == this->_locations[i].getErrorPages().end())
				this->_locations[i].addErrorPage(it->first, it->second);
		}
		if (this->_locations[i].getAllowMethods().size() == 0)
			this->_locations[i].setAllowMethods(this->_allowMethods);
		if (this->getIsRedirect())
		{
			this->_locations[i].setReturnURLOrBody(this->getReturnURLOrBody());
			this->_locations[i].setReturnCode(this->getReturnCode());
			this->_locations[i].setIsRedirect(true);
		}
		this->_locations[i].setMimeTypes(this->_mimeTypes);
		this->_locations[i].setPort(this->_port);
	}
}

void	ServerConfig::validateLocations(void)
{
	for (size_t i = 0; i < this->_locations.size(); i++)
	{
		resolveDotSegments(this->_locations[i].getPath(), CONFIG);
		resolveDotSegments(this->_locations[i].getRoot(), CONFIG);
		// validate location
		if (!this->_locations[i].getIsCgi()) // not CGI
		{
			// simple check for path validity, the rest of the path will be checked later with root and index file
			if (this->_locations[i].getPath()[0] != '/')
				throw(std::runtime_error("Config parser: Invalid location path."));
			if (!this->_locations[i].getIsRedirect())
			{
				// validate index (and path)
				for (size_t j = 0; j < this->_locations[i].getIndex().size(); j++)
					// Originally, the access was checked for root+path+/+index because that's what NGINX does. The subject asks us to replace the path with the root, so the path is left out of the check.
					//fileIsValidAndAccessible(resolveDotSegments(this->_locations[i].getRoot() + "/" + this->_locations[i].getIndex()[j], CONFIG), "Index");
					resolveDotSegments(this->_locations[i].getRoot() + "/" + this->_locations[i].getIndex()[j], CONFIG);
			}
		}
	}
}

int createSocket(void)
{
	int fdSocket;

	if ((fdSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		throw(std::runtime_error("Socket creation failed."));
	return (fdSocket);
}

void ServerConfig::bindSocket(void)
{
	int					yes = 1;

	if (setsockopt(this->_serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
		throw (std::runtime_error("Setsockopt failed."));
	memset(&this->_serverAddr, 0, sizeof(this->_serverAddr));
	this->_serverAddr.sin_family = AF_INET;
	this->_serverAddr.sin_addr.s_addr = this->_host;
	this->_serverAddr.sin_port = htons(this->_port);
	if (bind(this->_serverSocket, (struct sockaddr *)&this->_serverAddr, sizeof(this->_serverAddr)) == -1)
		throw (std::runtime_error("Socket binding failed."));
}

void ServerConfig::startServer(void)
{
	std::string	infoMessage;
	this->_serverSocket = createSocket();
	bindSocket();
	infoMessage = std::string("Server '") + this->_primaryServerName + "' started on " 
				+ itoa(this->_host % 256) + "." + itoa(this->_host / 256 % 256) + "."
				+ itoa(this->_host / 65536 % 256) + "." + itoa(this->_host / 16777216) + ":"
				+ itoa(this->_port);
	Logger::log(INFO, CONFIG, infoMessage, "");
}

std::ostream &operator << (std::ostream &o, ServerConfig const &instance)
{
	unsigned int					host;
	std::map<unsigned short, std::string>	errorPages;
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
	for (std::map<unsigned short, std::string>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it)
		o << it->first << ": " << it->second << '\n';
	o << "client_max_body_size (requestBodySizeLimit): " << instance.getRequestBodySizeLimit() << '\n'
		<< "autoindex: " << instance.getAutoindex() << '\n'
		<< "allow_methods: " << instance.getAllowMethods() << '\n'
		<< "return: " << instance.getReturnCode() << " " << instance.getReturnURLOrBody() << '\n'
		<< "mime_types: " << instance.getMimeTypeFile() << '\n';
	for (size_t i = 0; i < locations.size(); i++)
		o << locations[i] << '\n';
	return (o);
}
