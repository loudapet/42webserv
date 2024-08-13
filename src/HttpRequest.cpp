/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/09 09:56:07 by plouda            #+#    #+#             */
/*   Updated: 2024/08/12 17:21:09 by plouda           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/HttpRequest.hpp"
#include "../inc/HttpResponse.hpp"
#include "../inc/ResponseException.hpp"

HttpRequest::HttpRequest()
{
	Location		defaultLocation;
	HttpResponse	defaultResponse;
	this->requestLine.httpVersion = "1.1";
	this->requestLine.method = "";
	this->requestLine.requestTarget = (RequestTarget){
		.authority = std::make_pair("", ""),
		.absolutePath = "",
		.query = "",
		.fragment = ""
	};
	this->headerFields = stringmap_t(); 
	this->requestBody = octets_t();
	this->requestBodySizeLimit = REQUEST_BODY_SIZE_LIMIT;
	this->targetResource = "";
	this->cgiPathInfo = "";
	this->allowedMethods = std::set<std::string>();
	this->connectionStatus = KEEP_ALIVE;
	this->messageFraming = NO_CODING;
	this->allowedDirListing = false;
	this->isRedirect = false;
	this->contentLength = 0;
	this->location = defaultLocation;
	this->targetIsDirectory = false;

	this->response = defaultResponse;
	this->readingBodyInProgress = false;
	this->requestComplete = false;
	this->messageFraming = NO_CODING;
	this->connectionStatus = KEEP_ALIVE;
	this->hasExpect = false;
	this->silentErrorRaised = false;
	//this->requestID = 0;
	return ;
}

HttpRequest::HttpRequest(const HttpRequest& refObj)
{
	*this = refObj;
}

HttpRequest& HttpRequest::operator=(const HttpRequest& refObj)
{
	if (this != &refObj)
	{
		requestLine = refObj.requestLine;
		headerFields = refObj.headerFields;
		requestBody = refObj.requestBody;
		requestBodySizeLimit = refObj.requestBodySizeLimit;
		targetResource = refObj.targetResource;
		cgiPathInfo = refObj.cgiPathInfo;
		allowedMethods = refObj.allowedMethods;
		connectionStatus = refObj.connectionStatus;
		messageFraming = refObj.messageFraming;
		allowedDirListing = refObj.allowedDirListing;
		isRedirect = refObj.isRedirect;
		contentLength = refObj.contentLength;
		location = refObj.location;
		response = refObj.response;
		readingBodyInProgress = refObj.readingBodyInProgress;
		requestComplete = refObj.requestComplete;
		targetIsDirectory = refObj.targetIsDirectory;
		hasExpect = refObj.hasExpect;
		silentErrorRaised = refObj.silentErrorRaised;
		//requestID = refObj.requestID;
	}
	return (*this);
}

HttpRequest::~HttpRequest()
{
	return ;
}


void	HttpRequest::parseMethod(std::string& token)
{
	if (token == "GET" || token == "POST" || token == "DELETE")
		this->requestLine.method = token;
	else if (token == "HEAD" || token == "PUT" || token == "CONNECT"
			|| token == "OPTIONS" || token == "PATCH" || token == "TRACE")
		this->response.updateStatus(501, "Method not supported");
}

void resolvePercentEncoding(std::string& path, size_t& pos)
{
	if (path[pos] == '%')
	{
		if (pos + 2 < path.size())
		{	
			std::string	hexDigit = path.substr(pos + 1, 2);
			if (hexDigit.find_first_not_of(HEXDIGITS) != std::string::npos)
				throw(ResponseException(400, "Invalid percent encoding"));
			int	c = strtol(hexDigit.c_str(), NULL, 16);
			if (c)
			{
				path.erase(pos, 3);
				path.insert(pos, 1, static_cast<char>(c));
			}
			else
				throw(ResponseException(400, "Zero byte insertion detected"));
			pos++;
		}
		else					
			throw(ResponseException(400, "Invalid percent encoding"));
	}
}

void	HttpRequest::validateURIElements(void)
{
	size_t			pos = 0;
	std::string		extraAllowedChars("?#\0\0", 4);
	std::string		allowedChars(std::string(UNRESERVED) + std::string(PCHAR_EXTRA)
									+ std::string(SUBDELIMS));
	std::string*	elements[] = 	{&this->requestLine.requestTarget.absolutePath,
									&this->requestLine.requestTarget.query,
									&this->requestLine.requestTarget.fragment,
									&this->requestLine.requestTarget.authority.first};
	for (size_t i = 0; i < 4; i++)
	{
		pos = (*(elements)[i]).find_first_not_of(allowedChars);
		while (pos != std::string::npos && pos < (*(elements)[i]).size())
		{
			if ((*(elements)[i])[pos] == '%')
				resolvePercentEncoding(*(elements)[i], pos);
			else
				throw(ResponseException(400, "Invalid character in URI"));
			pos = (*(elements)[i]).find_first_not_of(allowedChars, pos);
		}
		allowedChars.push_back(extraAllowedChars[i]);
	}
}

stringpair_t	HttpRequest::parseAuthority(std::string& authority, HostLocation parseLocation)
{
	if (authority.find_first_of("@") != std::string::npos)
		throw(ResponseException(400, "'userinfo' component deprecated"));
	std::string		allowedChars(std::string(UNRESERVED) + std::string(SUBDELIMS) + std::string("%"));
	std::string		host("");
	std::string		port("");
	size_t			portPos = authority.find_first_of(":");
	int	i = 0;
	if (portPos == std::string::npos)
	{
		host = std::string(authority);
		if (host.size() == 0)
		{
			if (parseLocation == URI)
				throw(ResponseException(400, "Empty host name in URI"));
			else
				throw(ResponseException(400, "Empty host name in host header field"));
		}
		port = "";
	}
	else if (portPos == 0)
	{
		if (parseLocation == URI)
			throw(ResponseException(400, "Empty host name in URI"));
		else
			throw(ResponseException(400, "Empty host name in host header field"));
	}
	else
	{
		host = std::string(authority.begin(), authority.begin() + portPos);
		port = std::string(authority.begin() + portPos + 1, authority.end());
		if (port != "")
		{
			while (port[i] == '0') // delete leading zeros from port
				i++;
			port.erase(0, i);
		}	
	}
	if (host.find_first_not_of(allowedChars) != std::string::npos)
	{
		if (parseLocation == URI)
			throw(ResponseException(400, "Invalid host name in URI"));
		else
			throw(ResponseException(400, "Invalid host name in host header field"));
	}
	if (port.find_first_not_of(DIGITS) != std::string::npos || strtol(port.c_str(), NULL, 10) > 65535)
	{
		if (parseLocation == URI)
			throw(ResponseException(400, "Invalid port in URI"));
		else
			throw(ResponseException(400, "Invalid port in host header field"));
	}
	std::transform(host.begin(), host.end(), host.begin(), tolower); // case-insensitive
	return(std::make_pair(host, port));
}

void	HttpRequest::parseRequestTarget(std::string& uri)
{
	if (uri.size() > 8192)
		throw (ResponseException(414, "URI size should not exceed 8 kB"));
	if (uri.find_first_of('/') != 0)
	{
		std::string scheme(uri.begin(), uri.begin() + 7);
		std::transform(scheme.begin(), scheme.end(), scheme.begin(), tolower);
		if (scheme.find("http://") != 0)
			throw(ResponseException(400, "Invalid URI path"));
		else
		{
			std::string	delimiters("?#/");
			uri.erase(uri.begin(), uri.begin() + 7); // remove "http://"
			if (uri.size() == 0 || uri.find_first_of(delimiters) == 0)
				throw(ResponseException(400, "Authority required"));
			else
			{
				std::string	authority = std::string(uri.begin(), std::find_first_of(uri.begin(), uri.end(), delimiters.begin(), delimiters.end()));
				if (authority.size() == 0)
					throw(ResponseException(400, "Authority required"));
				this->requestLine.requestTarget.authority = this->parseAuthority(authority, URI);
				uri.erase(0, authority.size());
			}
		}
	}
	std::string::iterator	queryPos = std::find(uri.begin(), uri.end(), '?');
	std::string::iterator	fragmentPos = std::find(uri.begin(), uri.end(), '#');
	if (std::distance(uri.begin(), queryPos) > std::distance(uri.begin(), fragmentPos))
		queryPos = uri.end();
	if (queryPos == uri.end() && fragmentPos == uri.end())
	{
		this->requestLine.requestTarget.absolutePath = std::string(uri.begin(), uri.end());
		this->requestLine.requestTarget.query = std::string("");
		this->requestLine.requestTarget.fragment = std::string("");
	}
	else if (queryPos != uri.end() && fragmentPos != uri.end())
	{
		this->requestLine.requestTarget.absolutePath = std::string(uri.begin(), queryPos);
		this->requestLine.requestTarget.query = std::string(queryPos, fragmentPos);
		this->requestLine.requestTarget.fragment = std::string(fragmentPos, uri.end());
	}
	else if (queryPos != uri.end() && fragmentPos == uri.end())
	{
		this->requestLine.requestTarget.absolutePath = std::string(uri.begin(), queryPos);
		this->requestLine.requestTarget.query = std::string(queryPos, uri.end());
		this->requestLine.requestTarget.fragment = std::string("");
	}
	else if (queryPos == uri.end() && fragmentPos != uri.end())
	{
		this->requestLine.requestTarget.absolutePath = std::string(uri.begin(), fragmentPos);
		this->requestLine.requestTarget.query = std::string("");
		this->requestLine.requestTarget.fragment = std::string(fragmentPos, uri.end());
	}
	if (this->requestLine.requestTarget.absolutePath == "")
		this->requestLine.requestTarget.absolutePath = "/";
	validateURIElements();
	this->requestLine.requestTarget.absolutePath = resolveDotSegments(this->requestLine.requestTarget.absolutePath, REQUEST);
	return ;
}

void	HttpRequest::parseHttpVersion(std::string& token)
{
	std::string::iterator	slash = std::find(token.begin(), token.end(), '/');
	std::string	http(token.begin(), slash);
	if (http != "HTTP" || slash == token.end() || slash + 1 == token.end())
		throw(ResponseException(400, "Invalid protocol specification"));
	std::string	version(slash + 1, std::find_if(token.begin(), token.end(), isspace));
	if (version != "1.1")
	{
		if (version == "0.9")
			throw (ResponseException(426, "Supported HTTP versions: 1.0, 1.1"));
		else if (version != "1.0")
			throw (ResponseException(505, "Currently only 1.x supported"));
	}
	this->requestLine.httpVersion = version;
	if (version == "1.0")
		this->connectionStatus = CLOSE;
}

void	HttpRequest::parseRequestLine(std::string requestLine)
{
	std::vector<std::string>	startLineTokens;
	std::string::iterator		space = std::find(requestLine.begin(), requestLine.end(), SP);
	ParseToken					parse[3] = {&HttpRequest::parseMethod,
									&HttpRequest::parseRequestTarget,
									&HttpRequest::parseHttpVersion};

	while (space != requestLine.end() || requestLine.size() != 0) // size != 0 to grab the last segment
	{
		std::string token(requestLine.begin(), space);
		startLineTokens.push_back(token);
		while (space != requestLine.end() && isspace(*space))
			space++;
		requestLine.erase(requestLine.begin(), space);
		space = std::find(requestLine.begin(), requestLine.end(), SP);
	}
	if (startLineTokens.size() != 3)
		throw(ResponseException(400, "Invalid start line"));
	for (size_t i = 0; i < 3; i++)
		(this->*parse[i])(startLineTokens[i]);
}

void	HttpRequest::parseFieldSection(std::vector<std::string>& fields)
{
	std::string	allowedChars(std::string(DIGITS) + std::string(ALPHA) + std::string(TOKEN));
	std::string	fieldName;
	std::string	fieldValue;
	std::string::iterator	fieldIter;
	std::string::iterator	valueIter;
	stringmap_t::iterator	mapIter;
	for (std::vector<std::string>::iterator it = fields.begin() ; it != fields.end() ; it++)
	{
		if (it->size() > 8192)
			throw (ResponseException(413, "Header field size should not exceed 8 kB"));
		fieldIter = it->begin();
		if (it->find_first_of(':') == std::string::npos)
			throw (ResponseException(400, "Expected a field name"));
		std::advance(fieldIter, it->find_first_of(':'));
		fieldName = std::string(it->begin(), fieldIter);
		std::transform(fieldName.begin(), fieldName.end(), fieldName.begin(), tolower);
		if (fieldName.size() == 0 || fieldName.find_first_not_of(allowedChars.c_str()) != std::string::npos)
			throw(ResponseException(400, "Invalid field name"));
		fieldValue = std::string(++fieldIter, it->end()); // to remove ':' from the value part
		fieldValue = trim(fieldValue);
		valueIter = std::find_if(fieldValue.begin(), fieldValue.end(), std::not1(std::ptr_fun<int,int>(isgraph)));
		while (valueIter != fieldValue.end())
		{
			if (*valueIter >> 7) // UTF-8 encoding starts with 1 in its most-significant byte
			{
				valueIter = std::find_if(++valueIter, fieldValue.end(), std::not1(std::ptr_fun<int,int>(isgraph)));
				continue;
			}
			if (!isblank(*valueIter))
				throw(ResponseException(400, "Invalid field value - CTL characters forbidden"));
			else if (isblank(*valueIter) && isblank(*(valueIter + 1)))
				throw(ResponseException(400, "Invalid field value - multiple SP / HTAB detected"));
			valueIter = std::find_if(++valueIter, fieldValue.end(), std::not1(std::ptr_fun<int,int>(isgraph)));
		}
		if (!(this->headerFields.insert(std::make_pair(fieldName, fieldValue)).second)) // handle insertion of duplicates by concatenation
		{
			if (fieldName == "host")
				throw(ResponseException(400, "Duplicate Host header field"));
			mapIter = this->headerFields.find(fieldName);
			mapIter->second.append(std::string(",") + std::string(fieldValue));
		}
	}
	if (this->headerFields.find("host") == this->headerFields.end() && this->requestLine.httpVersion == "1.1")
		throw(ResponseException(400, "Missing Host header field"));
}

//host header field needs additional checks
// 1/ host from header needs to be parsed and percent-encoding has to be resolved (the latter is not specified by RFC but it's a nice-to-have feature)
// 2/ that does not need ot be done if there already is a host under requestLine.authority - but it should probably still be checked for invalid characters and separators
// 3/ identify comma-separated hosts (or automatically consider them as a singleton field)
stringpair_t	HttpRequest::resolveHost(void)
{
	stringpair_t			authority("","");
	if (this->headerFields.find("host") != this->headerFields.end()) // relevant for 1.0, where it is possible not to have a Host header field
	{
		stringmap_t::iterator	hostIter = this->headerFields.find("host");
		stringpair_t			hostHeader = this->parseAuthority(hostIter->second, HEADER_FIELD);
		size_t					pos = 0;
		pos = hostHeader.first.find('%');
		while (pos != std::string::npos && pos < hostHeader.first.size())
		{
			resolvePercentEncoding(hostHeader.first, pos);
			pos = hostHeader.first.find('%', pos);
		}
		if (hostHeader.first != "")
		{
			authority.first = hostHeader.first;
			authority.second = hostHeader.second;
		}
	}
	if (this->requestLine.requestTarget.authority.first != "")
	{
		authority.first = this->requestLine.requestTarget.authority.first;
		authority.second = this->requestLine.requestTarget.authority.second;
	}
	return (authority);
}

/* "In the interest of robustness, a server that is expecting to receive and parse a request-line
 SHOULD ignore at least one empty line (CRLF) received prior to the request-line." */
static void	trimHeaderEmptyLines(octets_t& header)
{
	octets_t::iterator	nl = std::find(header.begin(), header.end(), '\n');
	octets_t 			line(header.begin(), nl);
	while (header.size() > 0 && (line.size() == 0 || (line.size() == 1 && line[0] == '\r'))
			&& nl != header.end())
	{
		header.erase(header.begin(), nl + 1);
		nl = std::find(header.begin(), header.end(), '\n');
		line = octets_t(header.begin(), nl);
	}
}

/*
A sender MUST NOT generate a bare CR (a CR character not immediately followed by LF) 
within any protocol elements other than the content. A recipient of such a bare CR 
MUST consider that element to be invalid or replace each bare CR with SP
before processing the element or forwarding the message.
*/
static void	invalidateBareCR(octets_t& line)
{
	size_t	countCR = std::count(line.begin(), line.end(), '\r');
	if (countCR > 1)
		throw (ResponseException(400, "Bare CR detected"));
	else if (countCR == 1)
	{
		octets_t::iterator	cr = std::find(line.begin(), line.end(), '\r');
		if ((cr + 1) != line.end())
			throw (ResponseException(400, "Bare CR detected"));
	}
}

static void	invalidateNullBytes(octets_t& line)
{
	size_t	countNull = std::count(line.begin(), line.end(), '\0');
	if (countNull)
		throw (ResponseException(400, "Zero bytes disallowed"));
}

stringpair_t	HttpRequest::parseHeader(octets_t header)
{
	trimHeaderEmptyLines(header);
	std::vector<std::string>		splitLines;
	bool							startLine = true;
	bool							endLine = false;
	octets_t::iterator				nl = std::find(header.begin(), header.end(), '\n');
	octets_t 						line;
	stringpair_t					authority;
	while (nl != header.end())
	{
		octets_t line(header.begin(), nl);
		invalidateBareCR(line);
		invalidateNullBytes(line);
		if (line.size() == 0 || (line.size() == 1 && line[0] == '\r')) // indicates the end of field section
		{
			endLine = true;
			break ;
		}
		if (startLine)
			this->parseRequestLine(std::string(line.begin(), line.end()));
		if (!startLine)
			splitLines.push_back(std::string(line.begin(), line.end()));
		header.erase(header.begin(), nl + 1);
		nl = std::find(header.begin(), header.end(), '\n');
		if (startLine && isspace(*header.begin()))
			throw (ResponseException(400, "Dangerous whitespace detected"));
		startLine = false;
	}
	if (startLine)
		throw (ResponseException(400, "Empty start line"));
	if (nl == header.end() && !endLine) // the loop should only exit if there's a valid CRLF
		throw (ResponseException(400, "Missing empty CRLF"));
	if (std::distance(header.begin(), nl) == 1 && *header.begin() != '\r')
		throw (ResponseException(400, "Improperly terminated header-field section"));
	this->parseFieldSection(splitLines);
	authority = this->resolveHost();
	Logger::safeLog(DEBUG, REQUEST, "Resolved host:\t", authority.first);
	Logger::safeLog(DEBUG, REQUEST, "Resolved port:\t", authority.second);
	return (authority);
}

void	HttpRequest::validateContentType(const Location &location)
{
	stringmap_t::iterator											contentType;
	std::string														contentTypeValue;
	std::map< std::string, std::set<std::string> >					mimeTypesDict;
	// struct stat														statBuff;

	contentType = this->headerFields.find("content-type");
	mimeTypesDict = location.getMimeTypes().getMimeTypesDict();
	// if (stat((this->targetResource).c_str(), &statBuff) != 0)
	// 	throw (ResponseException(500, "Internal Server Error"));
	if (contentType != this->headerFields.end()) // content-type found in headerFields
	{
	// validate content-type header field
		contentTypeValue = splitQuotedString(contentType->second, ';')[0];
		contentTypeValue = trim(contentTypeValue);
		std::transform(contentTypeValue.begin(), contentTypeValue.end(), contentTypeValue.begin(), tolower); // case-insensitive
		Logger::safeLog(DEBUG, REQUEST, "Content type: ", contentTypeValue);
	    if (mimeTypesDict.find(contentTypeValue) == mimeTypesDict.end()) // content-type not found in mimeTypesDict
		{
			this->response.updateStatus(415, "Intended file type not supported");
			return ;
		}
	}
}

// has to be present before anything that can keep a persistent connection,
// e.g. Expect, 3xx responses
// handling contradicting values is implementation-specific; we just search for "close", otherwise the connection is kept alive - should be done the other way around for 1.0
// also checks for Keep-Alive header if close isn't specified
void	HttpRequest::validateConnectionOption(void)
{
	std::string							allowedChars(std::string(DIGITS) + std::string(ALPHA) + std::string(TOKEN) + "=;");
	stringmap_t::iterator				connection = this->headerFields.find("connection");
	stringmap_t::iterator				keepAlive = this->headerFields.find("keep-alive");
	std::vector<std::string>			values;
	std::vector<std::string>::iterator	option;
	std::vector<std::string>::iterator	param;
	size_t								invalidCharPos;
	//std::vector<std::string>			paramValues;
	if (connection != this->headerFields.end())
	{
		// if (std::count(connection->second.begin(), connection->second.end(), ';') > 1)
		// 	throw (ResponseException(400, "Invalid semicolons in Connection header field"));
		values = splitQuotedString(connection->second, ',');
		for (option = values.begin(); option != values.end(); option++)
		{
			*option = trim(*option);
			invalidCharPos = option->find_first_not_of(allowedChars);
			if (invalidCharPos != std::string::npos)
				throw (ResponseException(400, "Invalid characters in Connection header field"));
			std::transform(option->begin(), option->end(), option->begin(), tolower);
		}
		if (this->requestLine.httpVersion == "1.1")
		{
			if (std::find(values.begin(), values.end(), "close") != values.end())
				connectionStatus = CLOSE;
			else
				connectionStatus = KEEP_ALIVE;
		}
		else
		{
			if (std::find(values.begin(), values.end(), "keep-alive") != values.end() && this->requestLine.httpVersion == "1.0")
				connectionStatus = KEEP_ALIVE;
			else
				connectionStatus = CLOSE;
		}
	}
	else if (this->requestLine.httpVersion == "1.1")
		this->connectionStatus = KEEP_ALIVE;
	else
		this->connectionStatus = CLOSE;
	if (keepAlive != this->headerFields.end() && std::find(values.begin(), values.end(), "keep-alive") != values.end() 
			&& this->connectionStatus == KEEP_ALIVE) // only apply this when keep-alive option is set in Connection and close wasn't indicated
	{
		values = splitQuotedString(keepAlive->second, ',');
		for (param = values.begin(); param != values.end(); param++)
		{
			*param = trim(*param);
			if (param->find_first_not_of(allowedChars) != std::string::npos)
				throw (ResponseException(400, "Invalid characters in Keep-Alive header field"));
			std::transform(param->begin(), param->end(), param->begin(), tolower);
		}
	}
}

static void	removeDoubleSlash(std::string& str)
{
	size_t pos;
	while ((pos = str.find("//")) != std::string::npos)
		str.erase(pos, 1);
}

bool	isSupportedScript(std::string& path)
{
	std::string	ext;
	size_t pos = path.rfind('.');
	if (pos != std::string::npos)
	{
		ext = path.substr(pos + 1, path.length() - pos);
		if (ext == "py" || ext == "php" || ext == "cgi")
			return (true);
	}
	return (false);
}

void	HttpRequest::validateResourceAccess(const Location& location)
{
	bool		isCgi = location.getIsCgi();
	int			validFile;
	struct stat	fileCheckBuff;
	std::string	path = location.getPath();
	std::string	root = location.getRoot();
	if (*this->requestLine.requestTarget.absolutePath.rbegin() == '/' && *path.rbegin() != '/')
		path = path + "/";
	Logger::safeLog(DEBUG, REQUEST, "Location path:\t", path);
	Logger::safeLog(DEBUG, REQUEST, "Location root:\t", root);
	Logger::safeLog(DEBUG, REQUEST, "Absolute URL:\t", this->requestLine.requestTarget.absolutePath);
	std::size_t pos = this->requestLine.requestTarget.absolutePath.find(path);
	this->targetResource = this->requestLine.requestTarget.absolutePath;
	this->targetResource.replace(pos, path.length(), root);
	this->allowedDirListing = location.getAutoindex();
	removeDoubleSlash(this->targetResource);
	//std::cout << CLR3 << "CGI path:\t" << location.getRelativeCgiPath() << RESET << std::endl;
	if (!this->isRedirect && !isCgi) //&& this->requestLine.method == "GET"
	{
		validFile = stat(targetResource.c_str(), &fileCheckBuff);
		if (validFile < 0)
			this->response.updateStatus(404, "File does not exist");
		if (*targetResource.rbegin() != '/')
		{
			if (S_ISDIR(fileCheckBuff.st_mode)) // redirect to the existing folder
				this->response.updateStatus(301, "Trying to access a directory"); // DO NOT CHANGE THE STRING! (connected to prepareHeaders in Http::Response)
			else  // if it's a normal file, check access
			{
				if (access(targetResource.c_str(), R_OK) < 0)
					this->response.updateStatus(403, "Access forbidden");
			}
		}
		else if (*targetResource.rbegin() == '/') // target is a directory
		{
			std::vector<std::string>	pages = location.getIndex();
			bool						validIndexPage = false;
			for (size_t i = 0; i < pages.size(); i++)
			{
				if (access((this->targetResource + pages[i]).c_str(), R_OK | F_OK) == 0)
				{
					this->targetResource += pages[i];
					this->targetResource = resolveDotSegments(this->targetResource, REQUEST);
					if (this->targetResource.size() && this->targetResource[0] == '/')
						this->targetResource.erase(0, 1);
					validIndexPage = true;
					break ;
				}
			}
			if (!validIndexPage)
			{
				if (this->allowedDirListing)
				{
					DIR*	dirPtr;
					dirPtr = opendir(this->targetResource.c_str());
					if (dirPtr == NULL)
						this->response.updateStatus(500, "Failed to open directory");
					if (dirPtr != NULL && closedir(dirPtr))
						this->response.updateStatus(500, "Failed to close directory");
					this->targetIsDirectory = true;
				}
				else
					this->response.updateStatus(403, "Autoindexing is not allowed");
			}
		}
	}
	else if (!this->isRedirect && isCgi)
	{
		std::string	pathToResource = "";
		std::vector<std::string> segments = splitQuotedString(this->targetResource, '/');
		for (std::vector<std::string>::iterator it = segments.begin(); it != segments.end(); it++)
		{
			pathToResource.append(*it);
			validFile = stat(pathToResource.c_str(), &fileCheckBuff);
			if (validFile < 0)
			{
				this->response.updateStatus(404, "File does not exist");
				break;
			}
			else if (S_ISDIR(fileCheckBuff.st_mode))
			{
				pathToResource.append("/");
				continue;
			}
			else  // if it's a normal file, check extension validity and whether it can be executed
			{
				if (access(pathToResource.c_str(), X_OK) < 0)
				{
					this->response.updateStatus(403, "Not executable");
					break;
				}
				else if (!isSupportedScript(pathToResource))
				{
					this->response.updateStatus(404, "CGI script type not supported");
					break;
				}
				else
				{
					while (++it != segments.end())
					{
						this->cgiPathInfo.append("/");
						this->cgiPathInfo.append(*it);
					}
					if (*targetResource.rbegin() == '/')
						this->cgiPathInfo.append("/");
					this->targetResource = pathToResource;
					break;
				}
			}
		}
	}
	else if (this->isRedirect)
	{
		this->response.lockStatusCode();
		this->response.setStatusCode(location.getReturnCode());
		Logger::safeLog(INFO, REQUEST, "Return directive specifies return code: ", itoa(location.getReturnCode()));
	}
	Logger::safeLog(DEBUG, REQUEST, "Target resource:\t", this->targetResource);
	Logger::safeLog(DEBUG, REQUEST, "CGI PATH_INFO:\t", this->cgiPathInfo);
	// POST and DELETE
}

void	HttpRequest::validateMessageFraming(void)
{
	stringmap_t::iterator	transferEncoding = this->headerFields.find("transfer-encoding");
	stringmap_t::iterator	contentLength = this->headerFields.find("content-length");
	std::vector<std::string>	values;
	if (transferEncoding != this->headerFields.end() && this->requestLine.httpVersion == "1.0")
		throw (ResponseException(400, "Faulty message framing - TE disallowed in HTTP/1.0"));
	if (transferEncoding != this->headerFields.end() && contentLength != this->headerFields.end())
		throw (ResponseException(400, "Ambiguous message framing"));
	if (transferEncoding != this->headerFields.end())
	{
		values = splitQuotedString(transferEncoding->second, ',');
		for (std::vector<std::string>::iterator it = values.begin(); it != values.end(); it++)
		{
			*it = trim(*it);
			std::string type = splitQuotedString(*it, ';')[0]; // get rid of parameters
			if (type != "chunked" && type != "identity" && type != "")
				throw (ResponseException(501, "Unknown transfer-encoding"));
			this->messageFraming = TRANSFER_ENCODING;
		}
	}
	else if (contentLength != this->headerFields.end()) // needs reviewing
	{
		if (contentLength->second.find_first_not_of(DIGITS) != std::string::npos)
			throw (ResponseException(400, "Invalid Content-Length value"));
		errno = 0;
		long bodySize = strtol(contentLength->second.c_str(), NULL, 10);
		int	error = errno;
		if (error == ERANGE || bodySize > INT_MAX) // update to max_body_size
			throw (ResponseException(413, "Content-Length is too big"));
		this->contentLength = bodySize;
		this->messageFraming = CONTENT_LENGTH;
	}
	else
	{
		if (this->requestLine.method == "POST")
			throw (ResponseException(400, "Invalid framing (depends on method - FIX)"));
	}
}

void	HttpRequest::manageExpectations(void)
{
	stringmap_t::iterator	expectation = this->headerFields.find("expect");
	if (expectation != this->headerFields.end())
	{
		if (expectation->second != "100-continue")
			this->response.updateStatus(417, "");
		else if (this->response.getStatusCode() < 299)
		{
			this->hasExpect = true;
			this->readingBodyInProgress = true;
			throw (ResponseException(100, "Continue"));
		}
	}
}

/*
1) method check based on string comparison once the server block rules are matched, also needs 405 Method Not Allowed if applicable
2) merge paths and check access to URI based on allowed methods
Q: what happens if methods in config are not valid?
*/
void	HttpRequest::validateHeader(const Location& location)
{
	this->location = location;
	this->isRedirect = location.getIsRedirect();
	this->allowedMethods = location.getAllowMethods();
	this->requestBodySizeLimit = location.getRequestBodySizeLimit();

	if (std::find(allowedMethods.begin(), allowedMethods.end(), this->requestLine.method) == allowedMethods.end())
		this->response.updateStatus(405, "Method Not Allowed");
	this->validateConnectionOption();
	this->validateResourceAccess(location);
	this->validateContentType(location);
	this->validateMessageFraming();
	if (this->requestLine.httpVersion == "1.1")
		this->manageExpectations();
}

// also should check for valid characters in extension(?)
size_t	HttpRequest::readRequestBody(octets_t bufferedBody)
{
	size_t	bytesRead = 0;
	if (this->messageFraming == CONTENT_LENGTH && bufferedBody.size() < this->contentLength)
	{
		this->readingBodyInProgress = true;
		return (0);
	}
	if (this->messageFraming == CONTENT_LENGTH)
	{
		Logger::safeLog(DEBUG, REQUEST, "Content length: ", itoa(this->contentLength));
		Logger::safeLog(DEBUG, REQUEST, "Body size limit: ", itoa(this->requestBodySizeLimit));
		if (this->contentLength > static_cast<size_t>(this->requestBodySizeLimit))
			throw (ResponseException(413, "Payload too large"));
		this->requestBody = octets_t(bufferedBody.begin(), bufferedBody.begin() + this->contentLength);
		this->readingBodyInProgress = false;
		this->requestComplete = true;
		return (this->requestBody.size());
	}
	else if (this->messageFraming == TRANSFER_ENCODING)
	{
		Logger::safeLog(DEBUG, REQUEST, std::string("Received: ") + itoa(bufferedBody.size()) + " bytes", "");
		for (octets_t::iterator	it = bufferedBody.begin() ; it != bufferedBody.end() ; it = bufferedBody.begin())
		{
			octets_t::iterator	newline = std::find(bufferedBody.begin(), bufferedBody.end(), '\n');
			octets_t			chunkSizeLine(bufferedBody.begin(), newline);
			size_t				tempBytesRead = chunkSizeLine.size() + 1;
			octets_t::iterator	semicolon = std::find(chunkSizeLine.begin(), chunkSizeLine.end(), ';');
			octets_t			chunkSizeOct(chunkSizeLine.begin(), semicolon); // ignore chunk-extension
			bufferedBody.erase(bufferedBody.begin(), newline + 1); // move to the beginning of the chunk
			if (chunkSizeOct.size() > 0 && !isxdigit(chunkSizeOct[0]))
				throw (ResponseException(400, "Invalid chunk size value")); // anything else than a hexdigit at the start isn't allowed
			if (std::count(chunkSizeOct.begin(), chunkSizeOct.end(), '\0') > 0)
				throw (ResponseException(400, "Invalid chunk size value (null byte)")); // needs special check prior to conversion to string
			std::string	chunkSizeStr(chunkSizeOct.begin(), chunkSizeOct.end());
			chunkSizeStr = trim(chunkSizeStr);
			if (chunkSizeStr.find_first_not_of(HEXDIGITS) != std::string::npos)
				throw (ResponseException(400, "Invalid chunk size value"));
			errno = 0;
			size_t chunkSize = strtoul(chunkSizeStr.c_str(), NULL, 16);
			int	error = errno;
			if (error || this->requestBody.size() + chunkSize > static_cast<size_t>(this->requestBodySizeLimit) || chunkSize > INT_MAX)
				throw (ResponseException(413, "Payload too large"));

			if (chunkSize == 0)
			{
				if (bufferedBody.size() >= 1 && bufferedBody[0] == '\n')
				{
					bufferedBody.erase(bufferedBody.begin(), bufferedBody.begin() + 1);
					bytesRead += tempBytesRead + 1;
					this->readingBodyInProgress = false;
					this->requestComplete = true;
					return (bytesRead);
				}
				else if (bufferedBody.size() > 1 && bufferedBody[0] == '\r' && bufferedBody[1] == '\n')
				{
					bufferedBody.erase(bufferedBody.begin(), bufferedBody.begin() + 2);
					bytesRead += tempBytesRead + 2;
					this->readingBodyInProgress = false;
					this->requestComplete = true;
					return (bytesRead);
				}
				else
					throw (ResponseException(400, "Invalid delimitation of message body end"));
			}
			else if (chunkSize + 1 > bufferedBody.size()) // + 1 for newline (should be there as a proper delimiter)
			{
				this->readingBodyInProgress = true;
				return (bytesRead);
			}
			else
			{
				octets_t::iterator	itr = bufferedBody.begin();
				while (itr != bufferedBody.end() && static_cast<size_t>(std::distance(bufferedBody.begin(), itr)) < chunkSize)
				{
					this->requestBody.push_back(*itr);
					itr++;
				}						
				bufferedBody.erase(bufferedBody.begin(), itr);
				if (bufferedBody.size() >= 1)
				{
					if (bufferedBody.size() >= 1 && bufferedBody[0] == '\n')
					{
						bufferedBody.erase(bufferedBody.begin(), bufferedBody.begin() + 1);
						bytesRead++;
					}
					else if (bufferedBody.size() > 1 && bufferedBody[0] == '\r' && bufferedBody[1] == '\n')
					{
						bufferedBody.erase(bufferedBody.begin(), bufferedBody.begin() + 2);
						bytesRead += 2;
					}
					else
					{
						this->readingBodyInProgress = true;
						return (bytesRead);
					}
					bytesRead += tempBytesRead + chunkSize;
				}
				else
				{
					this->readingBodyInProgress = true;
					return (bytesRead);
				}
			}
		}
	}
	else // when body is not present
	{
		this->readingBodyInProgress = false;
		this->requestComplete = true;
	}
	return (bytesRead);
}

void	HttpRequest::setConnectionStatus(ConnectionStatus connectionStatus)
{
	this->connectionStatus = connectionStatus;
}

void	HttpRequest::setRequestBody(octets_t requestBody)
{
	this->requestBody = requestBody;
}

const std::string&			HttpRequest::getAbsolutePath(void) const
{
	return (this->requestLine.requestTarget.absolutePath);
}

const ConnectionStatus &HttpRequest::getConnectionStatus() const
{
	return (this->connectionStatus);
}

const std::set<std::string> &HttpRequest::getAllowedMethods() const
{
	return (this->allowedMethods);
}

const requestLine_t	&HttpRequest::getRequestLine(void) const
{
	return (this->requestLine);
}

const stringmap_t	&HttpRequest::getHeaderFields(void) const
{
	return (this->headerFields);
}

octets_t &HttpRequest::getRequestBody(void)
{
	return (this->requestBody);
}

const Location	&HttpRequest::getLocation(void) const
{
	return (this->location);
}

const std::string&	HttpRequest::getTargetResource() const
{
	return (this->targetResource);
}

const std::string&	HttpRequest::getCgiPathInfo() const
{
	return (this->cgiPathInfo);
}

const bool&	HttpRequest::getTargetIsDirectory() const
{
	return (this->targetIsDirectory);
}

// const int &HttpRequest::getRequestID() const
// {
// 	return (this->requestID);
// }

bool HttpRequest::getHasExpect() const
{
    return (this->hasExpect);
}

void	HttpRequest::disableHasExpect()
{
	this->hasExpect = false;
}

std::ostream &operator<<(std::ostream &os, const octets_t &vec)
{
	for (octets_t::const_iterator it = vec.begin(); it != vec.end(); it++)
		os << *it;
	return os;
}

std::ostream &operator<<(std::ostream &os, octets_t &vec)
{
	for (octets_t::iterator it = vec.begin(); it != vec.end(); it++)
		os << *it;
	return os;
}

std::ostream &operator<<(std::ostream &os, std::vector<octets_t> &vec)
{
	for (std::vector<octets_t>::iterator it = vec.begin(); it != vec.end(); it++)
		os << *it << std::endl;
	return os;
}

std::ostream &operator<<(std::ostream &os, std::vector<std::string> &vec)
{
	for (std::vector<std::string>::iterator it = vec.begin(); it != vec.end(); it++)
		os << *it << std::endl;
	return os;
}

std::ostream &operator<<(std::ostream &os, requestLine_t& requestLine)
{
	os << UNDERLINE << "Start line" << RESET << std::endl;
	os << "method: " << requestLine.method << std::endl;
	os << "request-target host: " << requestLine.requestTarget.authority.first << std::endl;
	os << "request-target port: " << requestLine.requestTarget.authority.second << std::endl;
	os << "request-target path: " << requestLine.requestTarget.absolutePath << std::endl;
	os << "request-target query: " << requestLine.requestTarget.query << std::endl;
	os << "request-target fragment: " << requestLine.requestTarget.fragment << std::endl;
	os << "http-version: " << requestLine.httpVersion << std::endl;
	return os;
}

std::ostream &operator<<(std::ostream &os, const requestLine_t& requestLine)
{
	os << UNDERLINE << "Start line" << RESET << std::endl;
	os << "method: " << requestLine.method << std::endl;
	os << "request-target host: " << requestLine.requestTarget.authority.first << std::endl;
	os << "request-target port: " << requestLine.requestTarget.authority.second << std::endl;
	os << "request-target path: " << requestLine.requestTarget.absolutePath << std::endl;
	os << "request-target query: " << requestLine.requestTarget.query << std::endl;
	os << "request-target fragment: " << requestLine.requestTarget.fragment << std::endl;
	os << "http-version: " << requestLine.httpVersion << std::endl;
	return os;
}

std::ostream &operator<<(std::ostream &os, std::map<std::string, std::string>& fieldSection)
{
	os << UNDERLINE << "Header fields" << RESET << std::endl;
	for (std::map<std::string, std::string>::iterator it = fieldSection.begin(); it != fieldSection.end(); it++)
		os << it->first << ": " << it->second << std::endl;
	return os;
}

void	HttpRequest::resetRequestObject(void)
{
	HttpRequest newRequest;
	HttpResponse newResponse;

	this->requestLine = newRequest.requestLine;
	this->headerFields = newRequest.headerFields;
	this->requestBody.clear();
	this->requestBodySizeLimit = newRequest.requestBodySizeLimit;
	this->targetResource.clear();
	this->cgiPathInfo.clear();
	this->connectionStatus = newRequest.connectionStatus;
	this->allowedDirListing = newRequest.allowedDirListing;
	this->isRedirect = newRequest.isRedirect;
	this->contentLength = newRequest.contentLength;
	this->messageFraming = newRequest.messageFraming;
	this->requestComplete = newRequest.requestComplete;
	this->readingBodyInProgress = newRequest.readingBodyInProgress;
	this->response = newResponse;
	this->targetIsDirectory = newRequest.targetIsDirectory;
	this->hasExpect = newRequest.hasExpect;
	this->location = newRequest.location;
	this->silentErrorRaised = newRequest.silentErrorRaised;
	//this->requestID = newRequest.requestID;
}