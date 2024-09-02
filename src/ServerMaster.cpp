/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerMaster.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/29 12:16:57 by aulicna           #+#    #+#             */
/*   Updated: 2024/09/02 15:11:34 by aulicna          ###   ########.fr       */
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
		Logger::log(WARNING, SERVER, "Received a terminating signal. Closed all connections and exiting.\n", "");
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
		}
		else
			Logger::setLogLevel(INFO);
		if (!logsFile.empty())
		{
			if ((fd = open(logsFile.c_str(), O_WRONLY | O_APPEND)) < 0)
				throw(std::runtime_error("Config parser: Logs file at '" + logsFile + "' could not be opened."));
			Logger::setOutputFd(fd);
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
		Logger::log(DEBUG, CONFIG, std::string("Server block ") + itoa(i) + this->_serverBlocks[i], "");
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
		Location generic(serverConfig);
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

static std::string	upper(std::string str)
{
	for (std::string::iterator c = str.begin(); str.end() != c; ++c)
		*c = *c == '-' ?  '_' : toupper(*c);
	return (str);
}

static std::string	httpUpper(std::string str)
{
	for (std::string::iterator c = str.begin(); str.end() != c; ++c)
		*c = *c == '-' ?  '_' : toupper(*c);
	return ("HTTP_" + str);
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
	std::string	path = cgi_location.getPath();
	std::string	root = cgi_location.getRoot();
	std::string	url = request.getCgiPathInfo();

	path_translated = url;
	url.erase(0, path.length());
	if (root.size() && *root.rbegin() == '/')
		root.erase(--(root.rbegin().base()));
	if (url.size() && *url.begin() == '/')
		url.erase((url.begin()));
	path_translated = root + "/" + url;
	return (path_translated);
}

static void	get_env(Client	&client, char **env)
{
	HttpRequest&	request = client.request;
	std::string		str;
	stringmap_t		envstrings;
	int				e = 0;

	for (stringmap_t::const_iterator it = request.getHeaderFields().begin(); it != request.getHeaderFields().end(); it++)
	{
		envstrings[httpUpper(it->first)] = it->second;
		envstrings[upper(it->first)] = it->second;
	}
	if (request.getRequestBody().size()) //message
		envstrings["CONTENT_LENGTH"] = itoa(request.getRequestBody().size());
	envstrings["GATEWAY_INTERFACE"] = "CGI/1.1";
	if (request.getCgiPathInfo().size())
	{
		envstrings["PATH_INFO"] = request.getCgiPathInfo();
		envstrings["PATH_TRANSLATED"] = getPathTranslated(client);
	}
	else
	{
		envstrings["PATH_INFO"] = "/" + request.getTargetResource();
		envstrings["REQUEST_URI"] = "/" + request.getTargetResource();
		envstrings["PATH_TRANSLATED"] = "/" + request.getTargetResource();
	}
	envstrings["QUERY_STRING"] = request.getRequestLine().requestTarget.query.size() > 1 ? request.getRequestLine().requestTarget.query.substr(1, request.getRequestLine().requestTarget.query.size() - 1) : std::string("");
	envstrings["REMOTE_ADDR"] = inet_ntoa(client.getClientAddr().sin_addr);
	envstrings["REMOTE_HOST"] = inet_ntoa(client.getClientAddr().sin_addr);;
	envstrings["REQUEST_METHOD"] = request.getRequestLine().method;
	envstrings["SCRIPT_NAME"] = request.getTargetResource(); //from path
	envstrings["SERVER_NAME"] = request.getLocation().getServerName();
	envstrings["SERVER_PORT"] = itoa(client.getServerConfig().getPort());
	envstrings["SERVER_PROTOCOL"] = "HTTP/" + request.getRequestLine().httpVersion;
	envstrings["SERVER_SOFTWARE"] = "webserv/nginx-but-better";
	envstrings["REMOTE_PORT"] = itoa(client.getClientAddr().sin_port);
	envstrings["HTTPS"] = "off"; 
	for (stringmap_t::iterator it = envstrings.begin(); it != envstrings.end(); it++)
	{
		str = it->first + "=" + it->second;
		env[e] = new char[str.size() + 1];
		std::copy(str.begin(), str.end(), env[e]);
		env[e][str.size()] = '\0';
		str.clear();
		e++;
	}
	env[e] = NULL;
}

void	ft_cgi(ServerMaster &sm, Client	&client)
{
	HttpRequest&	request = client.request;
	HttpResponse&	response = request.response;
	int				wpid;
	
	if (response.getCgiStatus() == NOCGI && (request.getLocation().getIsCgi() || request.getIsCgiExec()))
		response.setCgiStatus(CGI_STARTED);
	else if (response.getCgiStatus() == CGI_STARTED)
	{
		Logger::safeLog(DEBUG, REQUEST, "CGI started", "");
		int	pid;
		int	fd1[2]; // writing to child
		int	fd2[2]; // reading from child
		if ((request.getLocation().getIsCgi() && access(request.getTargetResource().c_str(), X_OK) != 0)
			|| (request.getIsCgiExec() && access(request.getLocation().getCgiExec().second.c_str(), X_OK) != 0))
		{
			response.setCgiStatus(CGI_ERROR);
			response.updateStatus(500, "Cannot access CGI file");
			return ;
		}
		if (pipe(fd1) == -1)
		{
			response.setCgiStatus(CGI_ERROR);
			response.updateStatus(500, "Piping failed");
			return ;
		}
		if (pipe(fd2) == -1 )
		{
			close(fd1[0]);
			close(fd1[1]);
			response.setCgiStatus(CGI_ERROR);
			response.updateStatus(500, "Piping failed");
			return ;
		}
		//fcntl(fd1[0], )
		if (fcntl(fd1[0], F_SETFL, O_NONBLOCK) < 0) // so that the pipes don't block each other in nested while on recv
		{
			response.setCgiStatus(CGI_ERROR);
			response.updateStatus(500, "FCNTL failed");
		}
		if (fcntl(fd1[1], F_SETFL, O_NONBLOCK) < 0) // so that the pipes don't block each other in nested while on recv
		{
			response.setCgiStatus(CGI_ERROR);
			response.updateStatus(500, "FCNTL failed");
		}
		if (fcntl(fd2[0], F_SETFL, O_NONBLOCK) < 0) // so that the pipes don't block each other in nested while on recv
		{
			response.setCgiStatus(CGI_ERROR);
			response.updateStatus(500, "FCNTL failed");
		}
		if (fcntl(fd2[1], F_SETFL, O_NONBLOCK) < 0) // so that the pipes don't block each other in nested while on recv
		{
			response.setCgiStatus(CGI_ERROR);
			response.updateStatus(500, "FCNTL failed");
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
				response.setCgiStatus(CGI_ERROR);
				response.updateStatus(500, "Forking failed");
				return ;
			}
			else if (pid == 0)
			{
				//child
				if (dup2 (fd1[0], STDIN_FILENO) < 0)
				{
					close (fd1[0]);
					close (fd1[1]);
					close (fd2[0]);
					close (fd2[1]);
					throw(std::runtime_error("Dup2 failed: " + std::string(strerror(errno))));
				}
				close (fd1[0]);
				close (fd1[1]);
				if (dup2 (fd2[1], STDOUT_FILENO) < 0)
				{
					close (fd2[0]);
					close (fd2[1]);
					throw(std::runtime_error("Dup2 failed: " + std::string(strerror(errno))));
				}
				close (fd2[0]);
				close (fd2[1]);
				char *env_vars[250];
				char **env = &env_vars[0];
				get_env(client, env);
				char *ex[2];
				ex[0] = (char *)request.getTargetResource().c_str();
				ex[1] = NULL;
				char **av = &ex[0];
				if (request.getIsCgiExec())
					execve(request.getLocation().getCgiExec().second.c_str(), av, env);
				else
					execve(request.getTargetResource().c_str(), av, env);
				for (int i = 0; env[i]; i++)
					delete env[i];
				throw(std::runtime_error("Execve failed: " + std::string(strerror(errno))));
			}
			else
			{
				response.setCgiStatus(CGI_READING);
				response.setCgiPid(pid);
				response.setWfd(fd1[1]);
				response.setRfd(fd2[0]);
				close(fd1[0]);
				close(fd2[1]);
				return ;
			}
		}
	}
	else if (response.getCgiStatus() == CGI_READING)
	{
		ssize_t			w = 0;
		size_t			wsize;
		unsigned char	wbuffer[CGI_BUFFER_SIZE];
		int				status;

		wpid = waitpid(response.getCgiPid(), &status, WNOHANG);
		if (sm.fdIsSetWrite(response.getWfd()))
		{
			wsize = request.getRequestBody().size();
			if (wsize > CGI_BUFFER_SIZE)
				wsize = CGI_BUFFER_SIZE;
			if (wsize)
			{
				std::copy(request.getRequestBody().begin(), request.getRequestBody().begin() + wsize, wbuffer);
				w = write(response.getWfd(), wbuffer, wsize);
				// Logger::safeLog(DEBUG, REQUEST, "CGI written:", itoa(w));
			}
			if (w > 0)
			{
				request.getRequestBody().erase(request.getRequestBody().begin(), request.getRequestBody().begin() + w);
				return ;
			}
		}
		ssize_t	r;
		uint8_t	buffer[CGI_BUFFER_SIZE];
		if (sm.fdIsSetRead(response.getRfd()))
		{
			r = read(response.getRfd(), buffer, CGI_BUFFER_SIZE);
			if (r > 0)
			{
				// Logger::safeLog(DEBUG, REQUEST, "CGI read:", itoa(r));
				for (int i = 0; i < r; i++)
				{
					response.getCgiBody().push_back(buffer[i]);
				}
			}
			if (response.getCgiBody().size() > MAX_FILE_SIZE)
			{
				response.setCgiStatus(CGI_ERROR);
				response.updateStatus(502, "CGI response too large");
				response.getCgiBody().clear();
				return ;
			}
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
					if ((*it[1] == '\n' && *it[2] == '\n') || (*it[0] == '\n' && *it[1] == '\r' && *it[2] == '\n'))
					{
						std::istringstream header(std::string(response.getCgiBody().begin(), it[1]));
						while (std::getline(header, line))
						{
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
							key.clear();
							value.clear();
						}
						response.getCgiBody().erase(response.getCgiBody().begin(), it[2] + 1);
						return ;
					}
					it[0]++;
					it[1]++;
					it[2]++;
				}
				return ;
			}
			else
			{
				response.setCgiStatus(CGI_ERROR);
				response.updateStatus(502, "CGI failure");
				response.getCgiBody().clear();
			}
			return ;
		}
	}
}

void	ft_post(ServerMaster &sm, Client &client)
{
	HttpRequest&	request = client.request;
	HttpResponse&	response = request.response;

	if (response.getPostStatus() == NOPOST)
	{
		int fd;
		
		if (!access(request.getTargetResource().c_str(), F_OK))
			response.setFileExists();
		fd = open(request.getTargetResource().c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (fd < 0)
		{
			response.setPostStatus(POST_ERROR);
			response.updateStatus(500, "Upload failure, could not open file");
		}
		response.setWfd(fd);
		response.setPostStatus(POST_STARTED);
	}
	else if (response.getPostStatus() == POST_WRITING)
	{
		size_t			w = 0;
		size_t			wsize;
		unsigned char	wbuffer[POST_BUFFER_SIZE];
		if (!sm.fdIsSetWrite(response.getWfd()))
			return ;
		wsize = request.getRequestBody().size();
		if (wsize > POST_BUFFER_SIZE)
			wsize = POST_BUFFER_SIZE;
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
			response.setPostStatus(POST_COMPLETE);
			return ;
		}
		else
		{
			response.setPostStatus(POST_ERROR);
			response.updateStatus(500, "Upload failure");
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
		{
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
				if (this->_servers.count(i)) // indicates that the server socket is ready to read which means that a client is attempting to connect
					acceptConnection(i);
				else if (this->_clients.count(i))
				{
					if (FD_ISSET(i, &readFds))
						handleDataFromClient(i);
					this->_clients.find(i)->second.bufferUnchecked = false;
					size_t	bytesToDelete = 0;
					if (this->_clients.find(i) == this->_clients.end())
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
								client.separateValidHeader(); // separates the header from the body, header is stored in receivedHeader, body in receivedData
								parserPair = client.request.parseHeader(client.getReceivedHeader());
								selectServerRules(parserPair, i); // resolve ServerConfig to HttpRequest
								client.clearReceivedHeader(); // clears request line and header fields
								const ServerConfig &serverConfig = client.getServerConfig();
								Logger::safeLog(INFO, REQUEST, "Authority: ", parserPair.first + ":" + itoa(client.getServerConfig().getPort()));
								client.request.validateHeader(matchLocation(client.request.getAbsolutePath(), serverConfig, parserPair.first));
								client.request.readingBodyInProgress = true;
							}
							if (client.request.readingBodyInProgress) // processing request body
							{
								bytesToDelete = client.request.readRequestBody(client.getReceivedData());									
								client.eraseRangeReceivedData(0, bytesToDelete);
								if (!client.request.requestComplete && bytesToDelete == 0) // waiting for the rest of the body as indicated by message framing - going back to select()
									break ;
							}
							if (client.request.requestComplete)
							{
								Logger::safeLog(DEBUG, REQUEST, "Request body size: ", itoa(client.request.getRequestBody().size()));
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
							removeFdFromSet(this->_readFds, i);
							addFdToSet(this->_writeFds, i);
							break ;
						}
					}
				}
			}
			else if (FD_ISSET(i, &writeFds) && this->_clients.count(i))
			{
				Client	&client = this->_clients.find(i)->second;
				if ((client.request.getLocation().getIsCgi() || client.request.getIsCgiExec()) && !client.request.getHasExpect() 
					&& (client.request.response.getStatusLine().statusCode >= 200 && client.request.response.getStatusLine().statusCode <= 299)
					&& !(client.request.getLocation().getIsRedirect()))
				{
					int old_cgi_status = client.request.response.getCgiStatus();
					ft_cgi(*this, client);
					int cgi_status = client.request.response.getCgiStatus();
					if (old_cgi_status == CGI_STARTED && cgi_status == CGI_READING)
					{
						addFdToSet(this->_readFds, client.request.response.getRfd());
						addFdToSet(this->_writeFds, client.request.response.getWfd());
					}
					else if (old_cgi_status == CGI_READING && cgi_status != CGI_READING)
					{
						close(client.request.response.getWfd());
						close(client.request.response.getRfd());
						removeFdFromSet(this->_writeFds, client.request.response.getWfd());
						removeFdFromSet(this->_readFds, client.request.response.getRfd());
						client.request.response.setWfd(0);
						client.request.response.setRfd(0);
						Logger::safeLog(DEBUG, REQUEST, "CGI complete: ", itoa(cgi_status));
					}
					if (cgi_status < CGI_COMPLETE)
						continue ;
				}
				if (!client.request.getLocation().getIsCgi()
					&& (client.request.getRequestLine().method == "POST" || client.request.getRequestLine().method == "PUT") 
					&& (client.request.response.getStatusLine().statusCode == 200)
					&& !(client.request.getLocation().getIsRedirect()))
				{
					int old_post_status = client.request.response.getPostStatus();
					ft_post(*this, client);
					int post_status = client.request.response.getPostStatus();
					if (post_status == POST_STARTED)
					{
						addFdToSet(this->_writeFds, client.request.response.getWfd());
						client.request.response.setPostStatus(POST_WRITING);
						continue ;
					}
					else if (post_status == POST_WRITING)
						continue ;
					else if (old_post_status== POST_WRITING && post_status != POST_WRITING)
					{
						close(client.request.response.getWfd());
						removeFdFromSet(this->_writeFds, client.request.response.getWfd());
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
				sendResult = send(i, buff, buffLen, 0);
				client.updateTimeLastMessage();
				if (sendResult == -1)
				{
					Logger::safeLog(WARNING, RESPONSE, "send() failed, ", "closing connection.");
					closeConnection(i);
				}
				else if (sendResult < static_cast<int>(buffLen))
					client.request.response.eraseRangeMessage(0, sendResult);
				else if (messageLen > CLIENT_MESSAGE_BUFF)
					client.request.response.eraseRangeMessage(0, buffLen);
				else
				{
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
			throw(ResponseException(421, "Port mismatch"));
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
	if (parserPair.first.empty()) // HTTP/1.0 support
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
}

void	ServerMaster::handleDataFromClient(const int clientSocket)
{
	uint8_t							recvBuf[CLIENT_MESSAGE_BUFF]; // Buffer to store received data
	ssize_t							bytesReceived;
	
	memset(recvBuf, 0, sizeof(recvBuf)); // clear the receive buffer
	Client &clientToHandle = this->_clients.find(clientSocket)->second; // reference to the client object
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
