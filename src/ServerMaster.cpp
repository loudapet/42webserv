/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerMaster.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/29 12:16:57 by aulicna           #+#    #+#             */
/*   Updated: 2024/07/25 13:53:41 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ServerMaster.hpp"
#include "../inc/ResponseException.hpp"
#include "../inc/webserv.hpp"

int ServerMaster::_connectionCounter = 0;

ServerMaster::ServerMaster(void)
{
	this->_fdMax = -1;
	FD_ZERO(&this->_readFds);
	FD_ZERO(&this->_writeFds);
}

void	ServerMaster::runWebserv(const std::string &configFile)
{
	std::ifstream		file;
	char				c;
	std::stringstream	tmpFileContent;
	std::set<int> 		ports;
	int					port;

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
	removeCommentsAndEmptyLines();
	detectServerBlocks();
	for (size_t i = 0; i < this->_serverBlocks.size(); i++)
	{
		ServerConfig newServer(this->_serverBlocks[i]);
		this->_serverConfigs.push_back(newServer);
	}
	for (size_t i = 0; i < this->_serverConfigs.size(); i++)
	{
		port = this->_serverConfigs[i].getPort();
		if (!ports.insert(port).second)
			throw(std::runtime_error("Config parser: Duplicate port detected."));
	}
	for (size_t i = 0; i < this->_serverConfigs.size(); i++)
		this->_serverConfigs[i].startServer(); // launch servers
	prepareServersToListen(); // listen
	listenForConnections(); // select
}

ServerMaster::~ServerMaster(void)
{
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
		this->_servers.erase(it2);
		it2 = this->_servers.begin();
	}
	if (!g_runWebserv)
	{
		std::cout << std::endl;
		Logger::log(WARNING, SERVER, "Received SIGINT. Closed all connections and exiting.", "");
	}
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
}


size_t	validateBlockStart(size_t pos, std::string &configContent, std::string scope)
{
	size_t	i;

	i = configContent.find_first_not_of(" \t\n\r", pos);
	if (i == std::string::npos)
		return (pos);
	if (configContent.substr(i, scope.size()) != scope)
		throw(std::runtime_error("Config parser: Invalid " + scope + " scope."));
	i += scope.size();
	i = configContent.find_first_not_of(" \t\n\r", i);
	if (i == std::string::npos || configContent[i] != '{')
		throw(std::runtime_error("Config parser: Invalid " + scope + " scope."));
	return (i);
}

size_t	validateBlockEnd(size_t pos, std::string &configContent)
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

void	processLogsBlock(std::string &logsBlock)
{
	std::vector<std::string>		logsBlockElements;
	std::string						logsLevel;
	std::string						logsFile;
	int								fd;

	logsBlockElements = splitBlock(logsBlock);
	for (size_t i = 0; i < logsBlockElements.size(); i++)
	{
		if (logsBlockElements[i] == "logs_level" && (i + 1) < logsBlockElements.size()
			&& validateElement(logsBlockElements[i + 1]))
		{
			if (logsLevel != "")
				throw(std::runtime_error("Config parser: Duplicate logs_level directive."));
			logsLevel = logsBlockElements[i + 1];
			i++;
		}
		else if (logsBlockElements[i] == "logs_file" && (i + 1) < logsBlockElements.size()
			&& validateElement(logsBlockElements[i + 1]))
		{
			if (logsFile != "")
				throw(std::runtime_error("Config parser: Duplicate logs_file directive."));
			logsFile = logsBlockElements[i + 1];
			i++;
		}
		else if (logsBlockElements[i] != "{" && logsBlockElements[i] != "}")
			throw (std::runtime_error("Config parser: Invalid directive in a logs block."));
	}
	if (!logsLevel.empty() || !logsFile.empty())
	{
		if (!logsLevel.empty())
		{
			std::transform(logsLevel.begin(), logsLevel.end(), logsLevel.begin(), ::toupper);
			if (std::find(Logger::getLevelArray().begin(), Logger::getLevelArray().end(), logsLevel) == Logger::getLevelArray().end())
				throw(std::runtime_error("Config parser: Invalid logs level."));
			Logger::setLogLevel(static_cast<LogLevel>(std::distance(Logger::getLevelArray().begin(), std::find(Logger::getLevelArray().begin(), Logger::getLevelArray().end(), logsLevel))));
			if (!logsFile.empty())
			{
				if ((fd = open(logsFile.c_str(), O_WRONLY | O_APPEND)) < 0)
					throw(std::runtime_error("Config parser: Logs file at '" + logsFile + "' could not be opened."));
				Logger::setOutputFd(fd);
			}
		}
		else
		{
			Logger::setLogLevel(DISABLED);
			if (!logsFile.empty())
			{
				if (access(logsFile.c_str(), F_OK) < 0)
					throw(std::runtime_error("Config parser: Logs file at '" + logsFile + "' does not exist."));
			}
		}
	}
}

void	ServerMaster::detectServerBlocks(void)
{
	size_t		blockStart;
	size_t		blockEnd;
	std::string	serverBlock;
	std::string	logsBlock;
	std::string::iterator itServer;
	std::string::iterator itLogs;

	if (this->_configContent.find("logs") != std::string::npos)
	{
		itServer = this->_configContent.begin() + this->_configContent.find("server");
		itLogs = this->_configContent.begin() + this->_configContent.find("logs");
		if (itLogs > itServer)
			throw(std::runtime_error("Config parser: Logs block must at the beginning of the config file."));
		blockStart = validateBlockStart(0, this->_configContent, "logs");
		blockEnd = validateBlockEnd(blockStart + 1, this->_configContent);
		logsBlock = this->_configContent.substr(blockStart, blockEnd - blockStart + 1);
		if (blockEnd == std::string::npos || blockEnd == blockStart + 1)
			throw(std::runtime_error("Config parser: Logs block has no scope."));
		this->_configContent.erase(0, blockEnd + 1);
		processLogsBlock(logsBlock);
	}
	if (this->_configContent.find("server") == std::string::npos)
		throw(std::runtime_error("Config parser: No server block found."));
	blockStart = validateBlockStart(0, this->_configContent, "server");
	blockEnd = validateBlockEnd(blockStart + 1, this->_configContent);
	if (blockEnd == std::string::npos || blockEnd == blockStart + 1)
		throw(std::runtime_error("Config parser: Server block has no scope."));
	while (blockStart < this->_configContent.length() - 1 && blockEnd != blockStart)
	{
		if (blockEnd == blockStart + 1)
			throw(std::runtime_error("Config parser: Server block has no scope."));
		serverBlock = this->_configContent.substr(blockStart, blockEnd - blockStart + 1);
		this->_serverBlocks.push_back(serverBlock);
		blockStart = validateBlockStart(blockEnd + 1, this->_configContent, "server");
		blockEnd = validateBlockEnd(blockStart + 1, this->_configContent);
	}
}

void	ServerMaster::printServerBlocks(void) const
{
	for (size_t i = 0; i < this->_serverBlocks.size(); i++)
	{
		Logger::log(DEBUG, CONFIG, std::string("Server block ") + itoa(i) + this->_serverBlocks[i], "");
		//	std::cout << "Server block " << i << ": " << std::endl;
		//	std::cout << this->_serverBlocks[i] << std::endl;
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
		Logger::log(DEBUG, SERVER, std::string("Server '") + this->_serverConfigs[i].getPrimaryServerName() + "' listening on port " + itoa(this->_serverConfigs[i].getPort()) + "...", "");
		//if (DEBUG)
		//	std::cout << "Server '" << this->_serverConfigs[i].getPrimaryServerName() << "' listening on port " << this->_serverConfigs[i].getPort() << "..." << std::endl;
	}
	this->_fdMax = this->_serverConfigs.back().getServerSocket();
}

const Location matchLocation(const std::string &absolutePath, const ServerConfig &serverConfig, const std::string &serverName)
{
	std::vector<Location>	locations;
	std::string				locationPath;
	size_t					bestMatchLength;
	size_t					bestMatchIndex;
	bool					match;
	std::set<std::string>	method;

	locations = serverConfig.getLocations();
	if (serverConfig.getIsRedirect())
	{
		Location generic(serverConfig.getReturnCode(), serverConfig.getReturnURLOrBody());
		return (generic);
	}
	bestMatchLength = 0;
	bestMatchIndex = 0;
	match = false;
	for(size_t i = 0; i < locations.size(); i++)
	{
		locationPath = locations[i].getPath();
		if (absolutePath.find(locationPath) == 0 && locationPath.length() > bestMatchLength)
		{
			if (locationPath == "/" || absolutePath[locationPath.length()] == '/'
				|| *locationPath.rbegin() == '/' || absolutePath.length() == locationPath.length())
			{
				bestMatchLength = locationPath.length();
				bestMatchIndex = i;
				match = true;
			}
		}
	}
	if (!match)
	{
		Location generic;
		return (generic);
	}
	locations[bestMatchIndex].setServerName(serverName);
	return (locations[bestMatchIndex]);
}

// https://www.rfc-editor.org/rfc/rfc3875#section-4.1
// "AUTH_TYPE"			//not needed? https://www.rfc-editor.org/rfc/rfc2617
// "CONTENT_LENGTH"		//The server MUST set this meta-variable if and only if the request is
						// accompanied by a message-body entity.  The CONTENT_LENGTH value must
						// reflect the length of the message-body after the server has removed
						// any transfer-codings or content-codings.
// "CONTENT_TYPE"		The server MUST set this meta-variable if an HTTP Content-Type field
						// is present in the client request header.  If the server receives a
						// request with an attached entity but no Content-Type header field, it
						// MAY attempt to determine the correct content type, otherwise it
						// should omit this meta-variable.
// "GATEWAY_INTERFACE"	//GATEWAY_INTERFACE = "CGI" "/" 1*digit "." 1*digit (1.1)
// "PATH_INFO"			The PATH_INFO variable specifies a path to be interpreted by the CGI
						// script.  It identifies the resource or sub-resource to be returned by
						// the CGI script, and is derived from the portion of the URI path
						// hierarchy following the part that identifies the script itself.
// "PATH_TRANSLATED"	http://somehost.com/cgi-bin/somescript/this%2eis%2epath%3binfo
// 							/this.is.the.path;info
// 						http://somehost.com/this.is.the.path%3binfo
// 							/usr/local/www/htdocs/this.is.the.path;info
// "QUERY_STRING"		The server MUST set this variable; if the Script-URI does not include
						// a query component, the QUERY_STRING MUST be defined as an empty
						// string ("").
// "REMOTE_ADDR"		//IPv.4
// "REMOTE_HOST"		The server SHOULD set this variable.  If the hostname is not
						// available for performance reasons or otherwise, the server MAY
						// substitute the REMOTE_ADDR value.
// "REMOTE_IDENT"		// not needed?
// "REMOTE_USER"		// not needed?
// "REQUEST_METHOD"		// GET POST HEAD + PUT DELETE token
// "SCRIPT_NAME"		The SCRIPT_NAME variable MUST be set to a URI path (not URL-encoded)
						// which could identify the CGI script
// "SERVER_NAME"		// localhost?
// "SERVER_PORT"		//8081
// "SERVER_PROTOCOL"	"HTTP" "/" 1*digit "." 1*digit (1.1)
// "SERVER_SOFTWARE"	The SERVER_SOFTWARE meta-variable MUST be set to the name and version
						// of the information server software making the CGI request (and
						// running the gateway).  It SHOULD be the same as the server
						// description reported to the client, if any.

// static std::string	to_string(size_t num)
// {
// 	std::stringstream	ss;

// 	ss << num;
// 	return (ss.str());
// }

// 7.2 META VARIABLES??? Go through getHeaderFields map? // except existing vatiables

static std::string	upper(std::string str)
{
	for (std::string::iterator c = str.begin(); str.end() != c; ++c)
		*c = *c == '-' ?  '_' : toupper(*c);
	return (str);
}

static std::string	lower(std::string str)
{
	for (std::string::iterator c = str.begin(); str.end() != c; ++c)
		*c = tolower(*c);
	return (str);
}

static std::string	getPathTranslated(Client &client)
{
	HttpRequest&	request = client.request;
	Location cgi_location = matchLocation(request.getCgiPathInfo(), client.getServerConfig(), request.getLocation().getServerName());

	
	std::string path_translated;
	// removeDoubleSlash(path_translated);
	// 	bool		isCgi = location.getIsCgi();
	// int			validFile;
	// struct stat	fileCheckBuff;
	std::string	path = cgi_location.getPath();
	std::string	root = cgi_location.getRoot();
	std::string	url = request.getCgiPathInfo();
	// if (*this->requestLine.requestTarget.absolutePath.rbegin() == '/' && *path.rbegin() != '/')
	// 	path = path + "/";
	std::cerr << CLR4 << "PATH:\t" << path << RESET << std::endl;
	std::cerr << CLR4 << "ROOT:\t" << root << RESET << std::endl;
	std::cerr << CLR4 << "URL:\t" << url << RESET << std::endl;
	path_translated = url;
	url.erase(0, path.length());
	if (root.size() && *root.rbegin() == '/')
		root.erase(--(root.rbegin().base())); //WTH??? https://stackoverflow.com/questions/1830158/how-to-call-erase-with-a-reverse-iterator
	if (url.size() && *url.begin() == '/')
		url.erase((url.begin()));
	path_translated = root + "/" + url;
	std::cerr << CLR4 << "FINAL:\t" << path_translated << RESET << std::endl;
	return (path_translated);
}

static void	get_env(Client	&client, char **env)
{
	HttpRequest&	request = client.request;
	// HttpResponse&	response = request.response;
	std::string		str;
	stringmap_t		envstrings;
	int				e = 0;

	if (request.getCgiPathInfo().size())
		
	for (stringmap_t::const_iterator it = request.getHeaderFields().begin(); it != request.getHeaderFields().end(); it++)
		envstrings[upper(it->first)] = it->second;
	// if (envstrings.find("AUTH_SCHEME") != envstrings.end())	//NOT SUPPORTED
	// 	envstrings["AUTH_TYPE"] = envstrings.find("AUTH_SCHEME");
	if (request.getRequestBody().size()) //message
		envstrings["CONTENT_LENGTH"] = itoa(request.getRequestBody().size());
	if (request.getHeaderFields().find("content-type") != request.getHeaderFields().end()) //Content type
		envstrings["CONTENT_TYPE"] = request.getHeaderFields().find("content-type")->second;
	envstrings["GATEWAY_INTERFACE"] = "CGI/1.1";
	if (request.getCgiPathInfo().size())
	{
		envstrings["PATH_INFO"] = request.getCgiPathInfo();
		envstrings["PATH_TRANSLATED"] = getPathTranslated(client);
	}
	envstrings["QUERY_STRING"] = request.getRequestLine().requestTarget.query.size() > 1 ? request.getRequestLine().requestTarget.query.substr(1, request.getRequestLine().requestTarget.query.size() - 1) : std::string("");
	envstrings["REMOTE_ADDR"] = inet_ntoa(client.getClientAddr().sin_addr);
	envstrings["REMOTE_HOST"] = inet_ntoa(client.getClientAddr().sin_addr);;
	// envstrings["REMOTE_IDENT=???"] = "";	// NOT SUPPORTED
	// envstrings["REMOTE_USER=???"] = "";	// NOT SUPPORTED
	envstrings["REQUEST_METHOD"] = request.getRequestLine().method;
	envstrings["SCRIPT_NAME"] = request.getTargetResource(); //from path
	envstrings["SERVER_NAME"] = request.getLocation().getServerName();
	//ServerConfig value??
	envstrings["SERVER_PORT"] = itoa(client.getServerConfig().getPort());
	envstrings["SERVER_PROTOCOL"] = "HTTP/" + request.getRequestLine().httpVersion;
	envstrings["SERVER_SOFTWARE"] = "webserv/nginx-but-better";
	if (request.getHeaderFields().find("user-agent") != request.getHeaderFields().end())
		envstrings["HTTP_USER_AGENT"] = request.getHeaderFields().find("user-agent")->second;
	envstrings["REMOTE_PORT"] = itoa(client.getClientAddr().sin_port);
	envstrings["HTTPS"] = "off"; // maybe not needed
	for (stringmap_t::iterator it = envstrings.begin(); it != envstrings.end(); it++)
	{
		str = it->first + "=" + it->second;
		env[e] = new char[str.size() + 1];
		std::copy(str.begin(), str.end(), env[e]);
		env[e][str.size()] = '\0';
		str.clear();
		e++;
	}
	std::cerr << "e is: " << e << std::endl;
	env[e] = NULL;
}

// Optional?
// https://www.cgi101.com/book/ch3/text.html
// DOCUMENT_ROOT	The root directory of your server
// HTTP_COOKIE		The visitor's cookie, if one is set
// HTTP_HOST		The hostname of the page being attempted
// HTTP_REFERER		The URL of the page that called your program
// HTTP_USER_AGENT	The browser type of the visitor
// HTTPS			"on" if the program is being called through a secure server
// PATH				The system path your server is running under
// ////QUERY_STRING	The query string (see GET, below)
// ////REMOTE_ADDR		The IP address of the visitor
// ////REMOTE_HOST		The hostname of the visitor (if your server has reverse-name-lookups on; otherwise this is the IP address again)
// REMOTE_PORT		The port the visitor is connected to on the web server
// ////REMOTE_USER		The visitor's username (for .htaccess-protected pages)
// ////REQUEST_METHOD	GET or POST
// REQUEST_URI		The interpreted pathname of the requested document or CGI (relative to the document root)
// SCRIPT_FILENAME	The full pathname of the current CGI
// ////SCRIPT_NAME		The interpreted pathname of the current CGI (relative to the document root)
// SERVER_ADMIN		The email address for your server's webmaster
// ////SERVER_NAME		Your server's fully qualified domain name (e.g. www.cgi101.com)
// ////SERVER_PORT		The port number your server is listening on
// ////SERVER_SOFTWARE	The server software you're using (e.g. Apache 1.3)

void	ft_cgi(ServerMaster &sm, Client	&client)
{
	HttpRequest&	request = client.request;
	HttpResponse&	response = request.response;
	int				wpid;
	// status code for CGI needs to be properly updated, I think?
	//std::cout << CLR6 << request.getLocation().getRelativeCgiPath() << RESET << std::endl;
	// this if might deserve its own function later
	//if (request.getLocation().getRelativeCgiPath().size())
	// std::cout << CLR6 "Processing CGI stuff0" RESET << std::endl;
	if (response.getCgiStatus() == NOCGI && request.getLocation().getIsCgi())
		response.setCgiStatus(CGI_STARTED);
	else if (response.getCgiStatus() == CGI_STARTED)
	{
		int	pid;
		int	fd1[2]; // writing to child
		int	fd2[2]; // reading from child
		if (access(request.getTargetResource().c_str(), X_OK) != 0)
		{
			std::cerr << "Error: access CGI" << std::endl;
			response.setCgiStatus(CGI_ERROR);
			return ;
		}
		if (pipe(fd1) == -1)
		{
			std::cerr << "Error: Pipe" << std::endl;
			response.setCgiStatus(CGI_ERROR);
			return ;
		}
		if (pipe(fd2) == -1 )
		{
			close(fd1[0]);
			close(fd1[1]);
			std::cerr << "Error: Pipe" << std::endl;
			response.setCgiStatus(CGI_ERROR);
			return ;
		}
		else
		{
			pid = fork();
			if (pid == -1)
			{
				close(fd1[0]);
				close(fd1[1]);
				close(fd2[0]);
				close(fd2[1]);
				std::cerr << "Error: Fork" << std::endl;
				response.setCgiStatus(CGI_ERROR);
				return ;
			}
			else if (pid == 0)
			{
				//child
				//some shenanigans to get execve working
				dup2 (fd1[0], STDIN_FILENO);
				close (fd1[0]);
				close (fd1[1]);
				dup2 (fd2[1], STDOUT_FILENO);
				close (fd2[0]);
				close (fd2[1]);
				char *env_vars[250];
				char **env = &env_vars[0];
				get_env(client, env);
				char *ex[2];
				//ex[0] = (char *)request.getLocation().getRelativeCgiPath().c_str();
				ex[0] = (char *)"test_cgi-bin/test.cgi";
				ex[1] = NULL;
				char **av = &ex[0];
				//execve(request.getLocation().getRelativeCgiPath().c_str(), av, env);
				execve(request.getTargetResource().c_str(), av, env);
				//clean exit later, get pid is not legal, maybe a better way to do it?
				for (int i = 0; env[i]; i++)
				{
					delete env[i];
				}
				std::cerr << "Failed to execute: " << ex[0] << std::endl;
				kill(getpid(), SIGINT);
				exit (1);
			}
			else
			{
				response.setCgiStatus(CGI_WRITING);
				response.setCgiPid(pid);
				response.setWfd(fd1[1]);
				response.setRfd(fd2[0]);
				//parent
				//close reading end of the first pipe
				close(fd1[0]);
				//close writing end of the second pipe
				close(fd2[1]);
				return ;
				
				// continue on 0 read
			}
		}
	}
	else if (response.getCgiStatus() == CGI_WRITING)
	{
		size_t			w = 0;
		size_t			wsize;
		//std::string body(request.getRequestBody().begin(), request.getRequestBody().end());
		unsigned char	wbuffer[CGI_BUFFER_SIZE];
		if (!sm.fdIsSetWrite(response.getWfd()))
			return ;
		wsize = request.getRequestBody().size();
		if (wsize > CGI_BUFFER_SIZE)
			wsize = CGI_BUFFER_SIZE;
		if (wsize)
		{
			std::copy(request.getRequestBody().begin(), request.getRequestBody().begin() + wsize, wbuffer);
			w = write(response.getWfd(), wbuffer, wsize);
		}
		if (w > 0)
		{
			request.getRequestBody().erase(request.getRequestBody().begin(), request.getRequestBody().begin() + w);
			return ;
		}
		else if (!wsize)
		{
			//std::cout << CLR6 "Written to CGI" RESET << std::endl;
			response.setCgiStatus(CGI_READING);
			return ;
		}
		else
		{
			//std::cerr << CLRE "write fail or nothing was written" RESET << std::endl;
			response.setCgiStatus(CGI_ERROR);
			return ;
		}
	}
	else if (response.getCgiStatus() == CGI_READING)
	{
		int	status;
		int	r;
		uint8_t	buffer[CGI_BUFFER_SIZE];
		// read needs to be in select somehow
		//what is read is sent?
		// close when read finished (<= 0)
		// fork and wait ? Make it non blocking
		// waitpid WNOHANG? flag for waiting for a response?
		if (!sm.fdIsSetRead(response.getRfd()))
			return ;
		wpid = waitpid(response.getCgiPid(), &status, WNOHANG);
		r = read(response.getRfd(), buffer, CGI_BUFFER_SIZE);
		if (r > 0)
		{
			// octets_t message;
			for (int i = 0; i < r; i++)
			{
				//there might be a better way
				response.getCgiBody().push_back(buffer[i]);
				// std::cout << CLR2 << "CGI MAIN STUFF" << response.getCgiBody().size() << RESET << std::endl;
			}
			//std::cout << CLR2 << "CGI MAIN STUFF" << response.getCgiBody().size() << RESET << std::endl;
			//std::cout << CLR2 << "if exited" << WIFEXITED(status) << RESET << std::endl;
			//std::cout << CLR2 << "status" << WEXITSTATUS(status) << RESET << std::endl;
			//std::cout << CLR2 << "if signalled" << WIFSIGNALED(status) << RESET << std::endl;
			//std::cout << CLR2 << "status" << WTERMSIG(status) << RESET << std::endl;
			// std::string str(response.getCgiBody().begin(), response.getCgiBody().end());
			// std::cout << str << std::endl;
			//std::cout << CLR6 "CGI Processed! " << r << RESET << std::endl;
		}
		else if (r < 0)
		{
			//std::cerr << CLRE "read fail or nothing was read" RESET << std::endl;
			response.setCgiStatus(CGI_ERROR);
			return ;
		}
		if (response.getCgiBody().size() > MAX_FILE_SIZE)
		{
			response.setCgiStatus(CGI_ERROR);
			return ;
		}
		if (wpid == response.getCgiPid())
		{
			if (WIFEXITED(status) && !WEXITSTATUS(status))
			{
				std::string			line;
				std::string			key;
				std::string			value;
				octets_t::iterator	it[4];
				response.setCgiStatus(CGI_COMPLETE);
				if (response.getCgiBody().size() < 4)
					return ;
				it[0] = response.getCgiBody().begin();
				it[1] = it[0];
				it[1]++;
				it[2] = it[1];
				it[2]++;
				while (it[2] != response.getCgiBody().end())
				{
					if ((*it[0] == '\n' && *it[1] == '\n') || (*it[0] == '\n' && *it[1] == '\r' && *it[2] == '\n'))
					{
						// std::string 
						std::istringstream header(std::string(response.getCgiBody().begin(), it[0]));
						// std::cout << "Header:" << header << std::endl;
						while (std::getline(header, line))
						{
							// std::cout << line << std::endl;
							line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
							if (line.find(':') != std::string::npos)
							{
								key = line.substr(0, line.find(':')) + ": ";
								value = line.substr(line.find(':') + 1, line.size());
								while (value[0] == ' ')
									value.erase(0, 1);
							}
							if (key.size() && value.size())
							{
								response.getCgiHeaderFields()[lower(key)] = value;
							}
							// std::cout << "key:" << key << std::endl;
							// std::cout << "value:" << value << std::endl;
							key.clear();
							value.clear();
						}
						if (*it[1] == '\n')
							response.getCgiBody().erase(response.getCgiBody().begin(), it[1] + 1);
						else
							response.getCgiBody().erase(response.getCgiBody().begin(), it[2] + 1);
						return ;
					}
					it[0]++;
					it[1]++;
					it[2]++;
				}
				//process response to header fields and body
				//find NLNL
				//Remove header from body
				//process header
				return ;
			}
			else
				response.setCgiStatus(CGI_ERROR);
			return ;
		}
	}
}

void	ServerMaster::listenForConnections(void)
{
	fd_set			readFds; // temp fds list for select()
	fd_set			writeFds; // temp fds list for select()
	struct timeval	selectTimer;
	stringpair_t	parserPair;
	int				sendResult;

	addFdToSet(this->_writeFds, Logger::getOutputFd());
	// main listening loop
	while(g_runWebserv)
	{
		selectTimer.tv_sec = 1;
		selectTimer.tv_usec = 0; // could be causing select to fail (with errno of invalid argument) if not set
		readFds = this->_readFds; // copy whole fds master list in the fds list for select (only listener socket in the first run)
		writeFds = this->_writeFds;
		if (select(this->_fdMax + 1, &readFds, &writeFds, NULL, &selectTimer) == -1)
		{ // QUESTION: is this errno according to subject?
			if (errno == EINTR) // prevents throwing an exception due to select being interrupted by SIGINT
				return ;
			throw(std::runtime_error("Select failed. + " + std::string(strerror(errno))));
		}
		// run through the existing connections looking for data to read
		for (int i = 0; i <= this->_fdMax; i++)
		{
			Logger::setActiveClient(i);
			if (FD_ISSET(i, &readFds) || (this->_clients.count(i) && this->_clients.find(i)->second.bufferUnchecked)) // finds a socket with data to read
			{
				Logger::safeLog(DEBUG, SERVER, "Active client fd: ", itoa(i));
				if (this->_servers.count(i)) // indicates that the server socket is ready to read which means that a client is attempting to connect
					acceptConnection(i);
				else if (this->_clients.count(i))
				{
					if (FD_ISSET(i, &readFds))
						handleDataFromClient(i);
					this->_clients.find(i)->second.bufferUnchecked = false;
					size_t	bytesToDelete = 0;
					if (this->_clients.find(i) == this->_clients.end()) // temp fix for a closed client
						continue;
					Client	&client = this->_clients.find(i)->second;
					while (client.getReceivedData().size() > 0 && this->_clients.find(i) != this->_clients.end()) // won't go back to select until it processes all the data in the buffer
					{
						try
						{	
							if (!client.request.requestComplete && !client.request.readingBodyInProgress && !client.hasValidHeaderEnd()) // client hasn't sent a valid header yet so we need to go back to select
								break ;
							if (!client.request.requestComplete && !client.request.readingBodyInProgress) // client has sent a valid header, this is the first while iteration, so we parse it
							{
								client.incrementRequestID();
								Logger::setActiveRequestID(client.getRequestID());
								client.separateValidHeader(); // separates the header from the body, header is stored in dataToParse, body in receivedData
								parserPair = client.request.parseHeader(client.getReceivedHeader());
								selectServerRules(parserPair, i); // resolve ServerConfig to HttpRequest
								client.clearReceivedHeader(); // clears request line and header fields

								// match location
								const ServerConfig &serverConfig = client.getServerConfig();
								client.request.validateHeader(matchLocation(client.request.getAbsolutePath(), serverConfig, parserPair.first));
								client.request.readingBodyInProgress = true;
							}
							//Logger::log(DEBUG, std::string("\nBody:\n") + std::string(client.getReceivedData().begin(), client.getReceivedData().end()), "");
							if (client.request.readingBodyInProgress) // processing request body
							{
								bytesToDelete = client.request.readRequestBody(client.getReceivedData());									
								client.eraseRangeReceivedData(0, bytesToDelete);
								if (!client.request.requestComplete && bytesToDelete == 0) // waiting for the rest of the body as indicated by message framing - going back to select()
									break ;
							}
							if (client.request.requestComplete)
							{
								Logger::safeLog(DEBUG, REQUEST, "Changing to send() mode on socket ", itoa(i));
								removeFdFromSet(this->_readFds, i);
								addFdToSet(this->_writeFds, i);
								break ;
							}
						}
						catch(const ResponseException& e) // this should be in the loop, in order not to close connection for 3xx status codes
						{
							if (e.getStatusLine().statusCode != 100)
							{
								client.request.response.setStatusLineAndDetails(e.getStatusLine(), e.getStatusDetails());
								client.request.setConnectionStatus(CLOSE);
							}
							Logger::safeLog(DEBUG, RESPONSE, "Changing to send() mode on socket ", itoa(i));
							removeFdFromSet(this->_readFds, i);
							addFdToSet(this->_writeFds, i);
							break ;
						}
					}
				}
			}
			else if (FD_ISSET(i, &writeFds) && this->_clients.count(i))
			{
				// CGI TBA - add conditions for it, othwerwise send normal response
				Client	&client = this->_clients.find(i)->second;
				if (client.request.getLocation().getIsCgi() && !client.request.getHasExpect() 
					&& (client.request.response.getStatusLine().statusCode >= 200 && client.request.response.getStatusLine().statusCode <= 299))
				{
					int old_cgi_status = client.request.response.getCgiStatus();
					//std::cout << CLR6 << "Old CGI status: " << old_cgi_status << RESET << std::endl;
					ft_cgi(*this, client);
					int cgi_status = client.request.response.getCgiStatus();
					//std::cout << CLR6 << "CGI status: " << cgi_status << RESET << std::endl;
					// # define CGI_STARTED 1
					// # define CGI_WRITING 2
					// # define CGI_READING 4
					// # define CGI_COMPLETE 8
					// # define CGI_ERROR 256
					if (old_cgi_status == CGI_STARTED && cgi_status == CGI_WRITING)
					{
						//std::cout << "Adding to read: " << client.request.response.getRfd() << std::endl;
						addFdToSet(this->_readFds, client.request.response.getRfd());
						//std::cout << "Adding to write: " << client.request.response.getWfd() << std::endl;
						addFdToSet(this->_writeFds, client.request.response.getWfd());
					}
					else if (old_cgi_status == CGI_WRITING && cgi_status == CGI_READING)
					{
						close(client.request.response.getWfd());
						removeFdFromSet(this->_writeFds, client.request.response.getWfd());
						client.request.response.setWfd(0);
					}
					else if (old_cgi_status == CGI_WRITING && cgi_status == CGI_ERROR)
					{
						close(client.request.response.getWfd());
						close(client.request.response.getRfd());
						removeFdFromSet(this->_writeFds, client.request.response.getWfd());
						removeFdFromSet(this->_readFds, client.request.response.getRfd());
						client.request.response.setWfd(0);
						client.request.response.setRfd(0);
					}
					else if (old_cgi_status == CGI_READING && cgi_status != CGI_READING)
					{
						close(client.request.response.getRfd());
						removeFdFromSet(this->_readFds, client.request.response.getRfd());
						client.request.response.setRfd(0);
					}
					if (cgi_status < CGI_COMPLETE)
					{
						continue ;
					}
				}
				if (!client.request.response.getMessageTooLongForOneSend())
					client.request.response.setMessage(client.request.response.prepareResponse(client.request));
				octets_t message = client.request.response.getMessage();
				size_t messageLen = message.size();
				Logger::safeLog(DEBUG, RESPONSE, "Response size: ", itoa(messageLen) + " bytes");
				size_t buffLen;
				if (messageLen <= CLIENT_MESSAGE_BUFF)
					buffLen = messageLen;
				else
				{
					buffLen = CLIENT_MESSAGE_BUFF;
					client.request.response.setMessageTooLongForOneSend(true);
				}
				char*	buff = new char [buffLen];
				for (size_t i = 0; i < buffLen; i++)
					buff[i] = message[i];
				//std::string buffStr(buff, buffLen); // prevents invalid read size from valgrind as buff is not null-terminated, it's a binary buffer so that we can send binery files too (e.g. executables)
				//std::cout << CLR4 << "SEND: " << buffStr << RESET << std::endl;
				//std::cout << "BUFF: " << client.getReceivedData() << std::endl;
				sendResult = send(i, buff, buffLen, 0);
				if (sendResult == -1)
				{
					Logger::safeLog(WARNING, RESPONSE, "send() failed, ", "closing connection.");
					//std::cerr << "Error sending acknowledgement to client." << std::endl;
					closeConnection(i);
				}
				else if (sendResult < static_cast<int>(buffLen))
					client.request.response.eraseRangeMessage(0, sendResult);
				else if (messageLen > CLIENT_MESSAGE_BUFF)
					client.request.response.eraseRangeMessage(0, buffLen);
				else
				{
					//Logger::safeLog(DEBUG, RESPONSE, "Bytes sent in response: ", itoa(sendResult));
					Logger::safeLog(DEBUG, RESPONSE, "Changing to recv() mode on socket ", itoa(i));
					if (client.getReceivedData().size() > 0) // ensures we get back to reading the buffer without needing to go through select()
						this->_clients.find(i)->second.bufferUnchecked = true;
					removeFdFromSet(this->_writeFds, i);
					addFdToSet(this->_readFds, i);
					if (!client.request.getHasExpect())
					{
						if (client.request.getConnectionStatus() == CLOSE)
							closeConnection(i);
						else
							client.request.resetRequestObject(); // reset request object for the next request, resetting requestComplete and readingBodyInProgress flags is particularly important
					}
					else
						client.request.disableHasExpect();
				}
				delete[] buff;
			}
			else if (FD_ISSET(i, &writeFds) && Logger::readyToWrite)
			{
				size_t	logSize = Logger::getLogBuffer().size();
				if (logSize > LOG_BUF)
				{
					write(Logger::getOutputFd(), Logger::getLogBuffer().c_str(), LOG_BUF);
					Logger::eraseLogRange(LOG_BUF);
				}
				else
				{
					write(Logger::getOutputFd(), Logger::getLogBuffer().c_str(), logSize);
					Logger::eraseLogRange(logSize);
					Logger::readyToWrite = false;
				}
			}
		}
		checkForTimeout();
	}
}

void ServerMaster::selectServerRules(stringpair_t parserPair, int clientSocket)
{
	unsigned short		portReceived;
	std::istringstream	iss;
	bool				match;
	struct sockaddr_in	sa;
	in_addr_t			hostReceived;
	in_addr_t				host;
	std::vector<std::string>	serverNames;
	std::map<int, ServerConfig>::const_iterator	it;
	int			fdServerConfig;

	match = false;
	if (parserPair.second.empty())
		portReceived = this->_clients.find(clientSocket)->second.getPortConnectedOn();
	else
	{
		iss.str(parserPair.second);
		if (!(iss >> portReceived) || !iss.eof())
			throw(std::runtime_error("Server rules: Port number is out of range for unsigned short."));
	}
	for (std::map<int, ServerConfig>::const_iterator it = this->_servers.begin(); it != this->_servers.end(); it++)
	{
		if (it->second.getPort() == portReceived && this->_clients.find(clientSocket)->second.getPortConnectedOn() == portReceived)
		{
			match = true;
			host = it->second.getHost();
			serverNames = it->second.getServerNames();
			fdServerConfig = it->first;
			break ;
		}
	}
	if (!match)
		throw(ResponseException(421, "Port mismatch"));
	if (parserPair.first.empty()) // HPTT/1.0 support
	{
		this->_clients.find(clientSocket)->second.setServerConfig(this->_servers.find(fdServerConfig)->second);
		return ;
	}
	if (inet_pton(AF_INET, parserPair.first.c_str(), &(sa.sin_addr)) > 0) // is host (IP address)
	{
		hostReceived = inet_addr(parserPair.first.data());
		if (hostReceived == host)
		{
			this->_clients.find(clientSocket)->second.setServerConfig(this->_servers.find(fdServerConfig)->second);
			//std::cout << "Chosen config for client on socket " << clientSocket << ": " << this->_clients.find(clientSocket)->second.getServerConfig() << std::endl;
	   		return ;
		}
		else
			throw(ResponseException(421, "IP address mismatch"));
	}
	else // is server_name
	{
		for (size_t i = 0; i < serverNames.size(); i++)
		{
			if (parserPair.first == serverNames[i])
			{
				this->_clients.find(clientSocket)->second.setServerConfig(this->_servers.find(fdServerConfig)->second);
				//std::cout << "Chosen config for client on socket " << clientSocket << ": " << this->_clients.find(clientSocket)->second.getServerConfig() << std::endl;
				return ;
			}
		}
		throw(ResponseException(421, "Server name mismatch"));
	}
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
	//std::cout << "Client connected to server port: " << newClient.getPortConnectedOn() << std::endl;
	Logger::safeLog(INFO, SERVER, "Client connected to server port: ", itoa(newClient.getPortConnectedOn()));
	newClient.updateTimeLastMessage();
	newClient.updateTimeLastValidHeaderEnd();
	addFdToSet(this->_readFds, clientSocket);
	if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) < 0) // so that the sockets don't block each other in nested while on recv
	{
		closeConnection(clientSocket);
		throw(std::runtime_error("Fcntl failed."));
	}
	newClient.setClientSocket(clientSocket);
	newClient.setClientAddr(clientAddr);
	this->_clients.insert(std::make_pair(clientSocket, newClient));
	Logger::safeLog(NOTICE, SERVER, std::string("New connection accepted from ") + inet_ntop(AF_INET, &clientAddr, buff, INET_ADDRSTRLEN),
		 std::string(". Assigned socket ") + itoa(clientSocket) + ".");
	ServerMaster::incrementConnectionCounter();
	Logger::mapFdToClientID(clientSocket);
	//std::cout << "New connection accepted from "
	//	<< inet_ntop(AF_INET, &clientAddr, buff, INET_ADDRSTRLEN)
	//	<< ". Assigned socket " << clientSocket << '.' << std::endl;
}

void	ServerMaster::handleDataFromClient(const int clientSocket)
{
	uint8_t							recvBuf[CLIENT_MESSAGE_BUFF]; // Buffer to store received data
	ssize_t							bytesReceived;
	
	memset(recvBuf, 0, sizeof(recvBuf)); // clear the receive buffer
	Client &clientToHandle = this->_clients.find(clientSocket)->second; // reference to the client object
	//if (((bytesReceived = recv(clientSocket, recvBuf, sizeof(recvBuf), 0)) <= 0) && clientToHandle.getReceivedData().size() == 0)
	if ((bytesReceived = recv(clientSocket, recvBuf, sizeof(recvBuf), 0)) <= 0)
	{
		if (bytesReceived == 0) // if the client has closed the connection
			Logger::safeLog(NOTICE, SERVER, std::string("Socket ") + itoa(clientSocket) + " hung up.", "");
		else if (bytesReceived < 0)  // if there was an error receiving data
			Logger::safeLog(NOTICE, SERVER, "Encountered an issue while receiving data from client", "");
		closeConnection(clientSocket);
	}
	else // if data has been received
	{
		clientToHandle.updateTimeLastMessage();
		clientToHandle.updateReceivedData(recvBuf, bytesReceived);
		clientToHandle.trimHeaderEmptyLines();
	}
}

void	ServerMaster::checkForTimeout(void)
{
	for (std::map<int, Client>::iterator it = this->_clients.begin(); it != this->_clients.end(); it++)
	{
		try
		{
			if (time(NULL) - it->second.getTimeLastMessage() > CONNECTION_TIMEOUT)
				throw(ResponseException(408, "Connection inactive for too long"));
			if (time(NULL) - it->second.getTimeLastValidHeaderEnd() > VALID_HEADER_TIMEOUT && it->second.getReceivedData().size() > 0)
				throw(ResponseException(408, "Request header timeout"));
		}
		catch(const ResponseException& e)
		{
			it->second.request.response.setStatusLineAndDetails(e.getStatusLine(), e.getStatusDetails());
			it->second.request.setConnectionStatus(CLOSE);
			removeFdFromSet(this->_readFds, it->second.getClientSocket());
			addFdToSet(this->_writeFds, it->second.getClientSocket());
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
	if (!g_runWebserv)
		Logger::log(NOTICE, SERVER, std::string("Connection closed on socket ") + itoa(clientSocket) + ".", "");
	else
		Logger::safeLog(NOTICE, SERVER, std::string("Connection closed on socket ") + itoa(clientSocket) + ".", "");
}

bool	ServerMaster::fdIsSetWrite(int fd) const
{
	return (FD_ISSET(fd, &this->_writeFds));
}

bool	ServerMaster::fdIsSetRead(int fd) const
{
	return (FD_ISSET(fd, &this->_readFds));
}

void	ServerMaster::incrementConnectionCounter()
{
	if (ServerMaster::_connectionCounter < INT_MAX)
		ServerMaster::_connectionCounter++;
	else
		ServerMaster::_connectionCounter = 1;
}

int	ServerMaster::getConnectionCounter()
{
	return (ServerMaster::_connectionCounter);
}
