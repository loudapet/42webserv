/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerMaster.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okraus <okraus@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/29 12:16:57 by aulicna           #+#    #+#             */
/*   Updated: 2024/07/18 16:55:19 by okraus           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ServerMaster.hpp"
#include "../inc/ResponseException.hpp"

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

//	if (DEBUG)
	//std::cout << "CONTENT (initial)" << std::endl;
//	if (DEBUG)
	//std::cout << this->_configContent << std::endl;
	removeCommentsAndEmptyLines();
	
	detectServerBlocks();
//	if (DEBUG)
	//std::cout << "DETECTED SERVER BLOCKS" << std::endl;
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
	prepareServersToListen(); // listen
	listenForConnections(); // select
}

ServerMaster::~ServerMaster(void)
{
	if (DEBUG)
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
		this->_servers.erase(it2);
		it2 = this->_servers.begin();
	}
	std::cout << "\nWarning: Received SIGINT. Closed all connections and exiting." << std::endl;
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
//	if (DEBUG)
	//std::cout << "CONTENT (removed comments)" << std::endl;
//	if (DEBUG)
	//std::cout << this->_configContent << std::endl;
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
//	if (DEBUG)
	//std::cout << "CONTENT (removed empty lines)" << std::endl;
//	if (DEBUG)
	//std::cout << this->_configContent << std::endl;
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
		if (DEBUG)
			std::cout << "Server block " << i << ": " << std::endl;
		if (DEBUG)
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
		if (DEBUG)
			std::cout << "Server '" << this->_serverConfigs[i].getPrimaryServerName() << "' listening on port " << this->_serverConfigs[i].getPort() << "..." << std::endl;
	}
	this->_fdMax = this->_serverConfigs.back().getServerSocket();
}

const Location matchLocation(const std::string &absolutePath, const std::vector<Location> &locations,
	bool serverIsRedirect, unsigned short serverReturnCode, const std::string &serverReturnURLOrBody)
{
	std::string locationPath;
	size_t		bestMatchLength;
	size_t		bestMatchIndex;
	bool		match;
	std::set<std::string> method;

	if (serverIsRedirect)
	{
		Location generic(serverReturnCode, serverReturnURLOrBody);
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
	for (std::string::iterator p = str.begin(); str.end() != p; ++p)
		*p = toupper(*p);
	return (str);
}

static void	get_env(HttpRequest& request, char **env)
{
	std::string			str;
	//stringmap_t			envstrings;
	int					e = 0;

	for (std::map<std::string, std::string>::const_iterator it = request.getHeaderFields().begin(); it != request.getHeaderFields().end(); it++)
	{
		str = upper(it->first) + "=" + it->second;
		env[e] = new char[str.size() + 1];
		std::copy(str.begin(), str.end(), env[e]);
		env[e][str.size()] = '\0';
		str.clear();
		e++;
	}
	str = "AUTH_TYPE=" "";
	env[e] = new char[str.size() + 1];
	std::copy(str.begin(), str.end(), env[e]);
	env[e][str.size()] = '\0';
	str.clear();
	e++;
	if (request.getRequestBody().size()) //message
	{
		str = "CONTENT_LENGTH=" + itoa(request.getRequestBody().size());
		env[e] = new char[str.size() + 1];
		std::copy(str.begin(), str.end(), env[e]);
		env[e][str.size()] = '\0';
		str.clear();
		e++;
	}
	if (request.getHeaderFields().find("content-type") != request.getHeaderFields().end()) //Content type
	{
		str = "CONTENT_TYPE=" + request.getHeaderFields().find("content-type")->second;
		// str = "CONTENT_TYPE=" + request.getHeaderFields()["content-type"];
		env[e] = new char[str.size() + 1];
		std::copy(str.begin(), str.end(), env[e]);
		env[e][str.size()] = '\0';
		str.clear();
		e++;
	}
	str = "GATEWAY_INTERFACE=CGI/1.1";
	env[e] = new char[str.size() + 1];
	std::copy(str.begin(), str.end(), env[e]);
	env[e][str.size()] = '\0';
	str.clear();
	e++;
	str = "PATH_INFO=";
	env[e] = new char[str.size() + 1];
	std::copy(str.begin(), str.end(), env[e]);
	env[e][str.size()] = '\0';
	str.clear();
	e++;
	str = "PATH_TRANSLATED=";
	env[e] = new char[str.size() + 1];
	std::copy(str.begin(), str.end(), env[e]);
	env[e][str.size()] = '\0';
	str.clear();
	e++;
	str = "QUERY_STRING=";
	env[e] = new char[str.size() + 1];
	std::copy(str.begin(), str.end(), env[e]);
	env[e][str.size()] = '\0';
	str.clear();
	e++;
	str = "REMOTE_ADDR=??? IPv4";
	env[e] = new char[str.size() + 1];
	std::copy(str.begin(), str.end(), env[e]);
	env[e][str.size()] = '\0';
	str.clear();
	e++;
	str = "REMOTE_HOST=ADDR?";
	env[e] = new char[str.size() + 1];
	std::copy(str.begin(), str.end(), env[e]);
	env[e][str.size()] = '\0';
	str.clear();
	e++;
	str = "REMOTE_IDENT=???";
	env[e] = new char[str.size() + 1];
	std::copy(str.begin(), str.end(), env[e]);
	env[e][str.size()] = '\0';
	str.clear();
	e++;
	str = "REMOTE_USER=???";
	env[e] = new char[str.size() + 1];
	std::copy(str.begin(), str.end(), env[e]);
	env[e][str.size()] = '\0';
	str.clear();
	e++;
	// std::cerr << CLR1 << request.getRequestLine() << RESET << std::endl;
	str = "REQUEST_METHOD=" + request.getRequestLine().method;
	env[e] = new char[str.size() + 1];
	std::copy(str.begin(), str.end(), env[e]);
	env[e][str.size()] = '\0';
	str.clear();
	e++;
	str = "SCRIPT_NAME=";
	env[e] = new char[str.size() + 1];
	std::copy(str.begin(), str.end(), env[e]);
	env[e][str.size()] = '\0';
	str.clear();
	e++;
		//ServerConfig value?? primary?
	str = "SERVER_NAME=localhost";
	env[e] = new char[str.size() + 1];
	std::copy(str.begin(), str.end(), env[e]);
	env[e][str.size()] = '\0';
	str.clear();
	e++;
	//ServerConfig value??
	str = "SERVER_PORT=8002";
	env[e] = new char[str.size() + 1];
	std::copy(str.begin(), str.end(), env[e]);
	env[e][str.size()] = '\0';
	str.clear();
	e++;
	str = "SERVER_PROTOCOL=HTTP/1.1";
	env[e] = new char[str.size() + 1];
	std::copy(str.begin(), str.end(), env[e]);
	env[e][str.size()] = '\0';
	str.clear();
	e++;
	str = "SERVER_SOFTWARE=ft_webserv";
	env[e] = new char[str.size() + 1];
	std::copy(str.begin(), str.end(), env[e]);
	env[e][str.size()] = '\0';
	str.clear();
	e++;
	str = "BONUS ENV BELOW";
	env[e] = new char[str.size() + 1];
	std::copy(str.begin(), str.end(), env[e]);
	env[e][str.size()] = '\0';
	str.clear();
	e++;
	if (request.getHeaderFields().find("user-agent") != request.getHeaderFields().end())
	{
		str = "HTTP_USER_AGENT=" + request.getHeaderFields().find("user-agent")->second;
		env[e] = new char[str.size() + 1];
		std::copy(str.begin(), str.end(), env[e]);
		env[e][str.size()] = '\0';
		str.clear();
		e++;
	}
	str = "REMOTE_PORT=";
	env[e] = new char[str.size() + 1];
	std::copy(str.begin(), str.end(), env[e]);
	env[e][str.size()] = '\0';
	str.clear();
	e++;
	str = "HTTPS=off";
	env[e] = new char[str.size() + 1];
	std::copy(str.begin(), str.end(), env[e]);
	env[e][str.size()] = '\0';
	str.clear();
	e++;
	str = "More?";
	env[e] = new char[str.size() + 1];
	std::copy(str.begin(), str.end(), env[e]);
	env[e][str.size()] = '\0';
	str.clear();
	e++;
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

void	ft_cgi(Client	&client)
{
	HttpRequest&	request = client.request;
	HttpResponse&	response = request.response;
	int				wpid;
	// status code for CGI needs to be properly updated, I think?
	//std::cout << CLR6 << request.getLocation().getRelativeCgiPath() << RESET << std::endl;
	// this if might deserve its own function later
	//if (request.getLocation().getRelativeCgiPath().size())
	std::cout << CLR6 "Processing CGI stuff0" RESET << std::endl;
	if (response.getCgiStatus() == NOCGI && request.getLocation().getIsCgi())
		response.setCgiStatus(CGI_STARTED);
	if (response.getCgiStatus() == CGI_STARTED)
	{
		std::cout << CLR6 "Processing CGI stuff 1" RESET << std::endl;
		int	pid;
		int	fd1[2]; // writing to child
		int	fd2[2]; // reading from child
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
			response.setWfd(fd1[1]);
			response.setRfd(fd2[0]);
			pid = fork();
			if (pid == -1)
			{
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
				get_env(request, env);
				char *ex[2];
				//ex[0] = (char *)request.getLocation().getRelativeCgiPath().c_str();
				ex[0] = (char *)"test_cgi-bin/test.cgi";
				ex[1] = NULL;
				char **av = &ex[0];
				//execve(request.getLocation().getRelativeCgiPath().c_str(), av, env);
				execve("test_cgi-bin/test1.cgi", av, env);
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
	if (response.getCgiStatus() == CGI_WRITING)
	{
		size_t	w;
		std::string body(request.getRequestBody().begin(), request.getRequestBody().end());
		w = write(response.getWfd(), body.c_str(), request.getRequestBody().size());
		if (w == request.getRequestBody().size())
		{
			std::cout << CLR6 "Written to CGI" RESET << std::endl;
			std::cout << CLR6 << body << RESET << std::endl;
			response.setCgiStatus(CGI_READING);
			return ;
		}
		else
		{
			std::cerr << CLRE "write fail or nothing was written" RESET << std::endl;
			response.setCgiStatus(CGI_ERROR);
			close(response.getWfd());
			close(response.getRfd());
			return ;
		}
	}
	if (response.getCgiStatus() == CGI_READING)
	{
		int	status;
		int	r;
		uint8_t	buffer[CGI_BUFFER_SIZE];
		// read needs to be in select somehow
		//what is read is sent?
		// close when read finished (<= 0)
		// fork and wait ? Make it non blocking
		// waitpid WNOHANG? flag for waiting for a response?
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
			std::cout << CLR2 << "CGI MAIN STUFF" << response.getCgiBody().size() << RESET << std::endl;
			std::cout << CLR2 << "if exited" << WIFEXITED(status) << RESET << std::endl;
			std::cout << CLR2 << "status" << WEXITSTATUS(status) << RESET << std::endl;
			std::cout << CLR2 << "if signalled" << WIFSIGNALED(status) << RESET << std::endl;
			std::cout << CLR2 << "status" << WTERMSIG(status) << RESET << std::endl;
			// std::string str(response.getCgiBody().begin(), response.getCgiBody().end());
			// std::cout << str << std::endl;
			std::cout << CLR6 "CGI Processed! " << r << RESET << std::endl;
		}
		else if (r < 0)
		{
			std::cerr << CLRE "read fail or nothing was read" RESET << std::endl;
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
				response.setCgiStatus(CGI_COMPLETE);
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
	
	
	// main listening loop
	while(g_runWebserv)
	{
		std::cout << CLR6 "server runs" RESET << std::endl;
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
			if (FD_ISSET(i, &readFds) || (this->_clients.count(i) && this->_clients.find(i)->second.bufferUnchecked)) // finds a socket with data to read
			{
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
					if (DEBUG)
						std::cout << client.getReceivedData().size() << std::endl;
					while (client.getReceivedData().size() > 0 && this->_clients.find(i) != this->_clients.end()) // won't go back to select until it processes all the data in the buffer
					{
						try
						{	
							if (!client.request.requestComplete && !client.request.readingBodyInProgress && !client.hasValidHeaderEnd()) // client hasn't sent a valid header yet so we need to go back to select
								break ;
							if (!client.request.requestComplete && !client.request.readingBodyInProgress) // client has sent a valid header, this is the first while iteration, so we parse it
							{
								client.separateValidHeader(); // separates the header from the body, header is stored in dataToParse, body in receivedData
								parserPair = client.request.parseHeader(client.getReceivedHeader());
								selectServerRules(parserPair, i); // resolve ServerConfig to HttpRequest
								client.clearReceivedHeader(); // clears request line and header fields

								// match location
								const ServerConfig &serverConfig = client.getServerConfig();
								client.request.validateHeader(matchLocation( client.request.getAbsolutePath(), serverConfig.getLocations(),
									serverConfig.getIsRedirect(), serverConfig.getReturnCode(), serverConfig.getReturnURLOrBody()));
								client.request.readingBodyInProgress = true;
								/* if (client.request.getHasExpect()) 
									throw (ResponseException(100, "Continue")); - moved to HttpRequest */
							}
							if (DEBUG)
								std::cout << "Body:\n" << client.getReceivedData() << RESET << std::endl;
							if (client.request.readingBodyInProgress) // processing request body
							{
								bytesToDelete = client.request.readRequestBody(client.getReceivedData());									
								client.eraseRangeReceivedData(0, bytesToDelete);
								if (!client.request.requestComplete && bytesToDelete == 0) // waiting for the rest of the body as indicated by message framing - going back to select()
									break ;
							}
							if (client.request.requestComplete)
							{
								if (DEBUG)
									std::cout << "Changing to send() " << i << std::endl;
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
							if (DEBUG)
								std::cout << "Changing to send() " << i << std::endl;
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
				std::cout << CLR6 "Processing CGI stuff -1" RESET << std::endl;
				if (client.request.getLocation().getIsCgi())
				{
					std::cout << CLR6 "Processing CGI stuff 0000" RESET << std::endl;
					int old_cgi_status = client.request.response.getCgiStatus();
					ft_cgi(client);
					int cgi_status = client.request.response.getCgiStatus();
					std::cout << CLRE << "CGI status: " << cgi_status << RESET << std::endl;
					// # define CGI_STARTED 1
					// # define CGI_WRITING 2
					// # define CGI_READING 4
					// # define CGI_COMPLETE 8
					// # define CGI_ERROR 256
					if (old_cgi_status == CGI_STARTED && cgi_status == CGI_WRITING)
					{
						addFdToSet(this->_readFds, client.request.response.getRfd());
						addFdToSet(this->_writeFds, client.request.response.getWfd());
					}
					else if (old_cgi_status == CGI_WRITING && cgi_status == CGI_READING)
					{
						close(client.request.response.getWfd());
						removeFdFromSet(this->_writeFds, client.request.response.getWfd());
					}
					else if (old_cgi_status == CGI_WRITING && cgi_status == CGI_ERROR)
					{
						removeFdFromSet(this->_writeFds, client.request.response.getWfd());
						removeFdFromSet(this->_readFds, client.request.response.getRfd());
					}
					else if (old_cgi_status == CGI_READING && cgi_status != CGI_READING)
					{
						close(client.request.response.getRfd());
						removeFdFromSet(this->_readFds, client.request.response.getRfd());
					}
					if (cgi_status < CGI_COMPLETE)
					{
						continue ;
					}
				}
				if(!client.request.response.getMessageTooLongForOneSend())
					client.request.response.setMessage(client.request.response.prepareResponse(client.request));
				octets_t message = client.request.response.getMessage();
				size_t messageLen = message.size();
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
				if (DEBUG)
					std::cout << "\033[31m" << "Bytes sent: " << sendResult << RESET << std::endl;
				if (sendResult == -1)
				{
					std::cerr << "Error sending acknowledgement to client." << std::endl;
					closeConnection(i);
				}
				else if (sendResult < static_cast<int>(buffLen))
					client.request.response.eraseRangeMessage(0, sendResult);
				else if (messageLen > CLIENT_MESSAGE_BUFF)
				{
					if (DEBUG)
						std::cout << "\033[31m" << "message too long for 8 KB buffer" << RESET << std::endl;
					if (DEBUG)
						std::cout << "Message size before erase of buffLen: " << client.request.response.getMessage().size() << std::endl;
					client.request.response.eraseRangeMessage(0, buffLen);
					if (DEBUG)
						std::cout << "Message size after erase of buffLen: " << client.request.response.getMessage().size() << std::endl;
				}
				else
				{
					std::cout << "Changing to recv() " << i << std::endl;
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
			if (DEBUG)
				std::cout << "Choosen config for client on socket " << clientSocket << ": " << this->_clients.find(clientSocket)->second.getServerConfig() << std::endl;
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
				if (DEBUG)
					std::cout << "Choosen config for client on socket " << clientSocket << ": " << this->_clients.find(clientSocket)->second.getServerConfig() << std::endl;
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
	std::cout << "Client connected to server port: " << newClient.getPortConnectedOn() << std::endl;
	newClient.updateTimeLastMessage();
	newClient.updateTimeLastValidHeaderEnd();
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
	//const char						*confirmReceived = "Well received!\n";
	ssize_t							bytesReceived;
	
	memset(recvBuf, 0, sizeof(recvBuf)); // clear the receive buffer
	Client &clientToHandle = this->_clients.find(clientSocket)->second; // reference to the client object
	//if (((bytesReceived = recv(clientSocket, recvBuf, sizeof(recvBuf), 0)) <= 0) && clientToHandle.getReceivedData().size() == 0)
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
		if (DEBUG)
			std::cout << "Data from client on socket " << clientSocket << ": ";
		if (DEBUG)
			clientToHandle.printReceivedData();
		if (DEBUG)
			std::cout << std::endl;
		
		// send acknowledgement to the client 
		// if (send(clientSocket, confirmReceived, strlen(confirmReceived), 0) == -1)
		// 	std::cerr << "Error sending acknowledgement to client." << std::endl;
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
	std::cout << "Connection closed on socket " << clientSocket << "." << std::endl;
}

bool	ServerMaster::fdIsSetWrite(int fd) const
{
	return (FD_ISSET(fd, &this->_writeFds));
}

bool	ServerMaster::fdIsSetRead(int fd) const
{
	return (FD_ISSET(fd, &this->_readFds));
}
