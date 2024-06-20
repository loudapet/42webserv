/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/09 09:56:07 by plouda            #+#    #+#             */
/*   Updated: 2024/06/19 15:14:03 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/HttpRequest.hpp"
#include "../inc/HttpResponse.hpp"
#include "../inc/ResponseException.hpp"


std::string	root = "./docs";

HttpRequest::HttpRequest()
{
	this->readingBodyInProgress = false;
	this->requestComplete = false;
	this->messageFraming = NO_CODING;
	this->connectionStatus = KEEP_ALIVE;
	return ;
}

HttpRequest::HttpRequest(const HttpRequest& refObj)
{
	*this = refObj;
}

HttpRequest& HttpRequest::operator = (const HttpRequest& refObj)
{
	(void)refObj;
	return (*this);
}

HttpRequest::~HttpRequest()
{
	return ;
}

std::string	trim(const std::string& str)
{
	const std::string	whitespace(" \t\r\n");
	const size_t		strBegin = str.find_first_not_of(whitespace);
	if (strBegin == std::string::npos)
		return (""); // no content
	const size_t strEnd = str.find_last_not_of(whitespace);
	const size_t strRange = strEnd - strBegin + 1;
	return (str.substr(strBegin, strRange));
}

// upgrade to handle backslashes and quotes as literals
std::vector<std::string>	splitQuotedString(const std::string& str, char sep)
{
	if (std::count(str.begin(), str.end(), '\"') % 2)
		throw(ResponseException(400, "Unclosed quotes in quoted string"));
	std::vector<std::string> splitString;
	unsigned int counter = 0;
	std::string segment;
	std::stringstream stream_input(str);
	while(std::getline(stream_input, segment, '\"'))
	{
		++counter;
		if (counter % 2 == 0)
		{
			if (!segment.empty())
				splitString.push_back(segment);
		}
		else
		{
			std::stringstream stream_segment(segment);
			while(std::getline(stream_segment, segment, sep))
				if (!segment.empty())
					splitString.push_back(segment);
		}
	}
	return (splitString);
}

void	HttpRequest::parseMethod(std::string& token)
{
	if (token == "GET" || token == "POST" || token == "DELETE")
		this->requestLine.method = token;
	else if (token == "HEAD" || token == "PUT" || token == "CONNECT"
			|| token == "OPTIONS" || token == "PATCH" || token == "TRACE")
		throw (ResponseException(501, "Method not supported"));
	else
		throw (ResponseException(400, "Unknown method name"));
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
	//the following two lines should be in the constructor
	this->requestLine.requestTarget.authority.first = "";
	this->requestLine.requestTarget.authority.second = "";
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

// pending response to invalid versions and to version 1.0
void	HttpRequest::parseHttpVersion(std::string& token)
{
	std::string::iterator	slash = std::find(token.begin(), token.end(), '/');
	std::string	http(token.begin(), slash);
	if (http != "HTTP" || slash == token.end() || slash + 1 == token.end())
		throw(ResponseException(400, "Invalid protocol specification"));
	std::string	version(slash + 1, std::find_if(token.begin(), token.end(), isspace));
	if (version != "1.1")
		throw(ResponseException(505, "Currently only 1.1 supported"));
	this->requestLine.httpVersion = token;
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
		fieldIter = it->begin();
		std::advance(fieldIter, it->find_first_of(':'));
		fieldName = std::string(it->begin(), fieldIter);
		std::transform(fieldName.begin(), fieldName.end(), fieldName.begin(), tolower);
		if (fieldName.size() == 0 || fieldName.find_first_not_of(allowedChars.c_str()) != std::string::npos)
			throw(ResponseException(400, "Invalid field name"));
		fieldValue = std::string(++fieldIter, it->end()); // to remove ':' from the value part
		fieldValue = trim(fieldValue);
		//valueIter = std::find_if_not(fieldValue.begin(), fieldValue.end(), isgraph);
		valueIter = std::find_if(fieldValue.begin(), fieldValue.end(), std::not1(std::ptr_fun<int,int>(isgraph)));
		while (valueIter != fieldValue.end())
		{
			/* std::bitset<8> x(*valueIter);
			std::cout << x << std::endl; */
			if (*valueIter >> 7) // UTF-8 encoding starts with 1 in its most-significant byte
			{
				//valueIter = std::find_if_not(++valueIter, fieldValue.end(), isgraph);
				valueIter = std::find_if(++valueIter, fieldValue.end(), std::not1(std::ptr_fun<int,int>(isgraph)));
				continue;
			}
			if (!isblank(*valueIter))
				throw(ResponseException(400, "Invalid field value - CTL characters forbidden"));
			else if (isblank(*valueIter) && isblank(*(valueIter + 1)))
				throw(ResponseException(400, "Invalid field value - multiple SP / HTAB detected"));
			//valueIter = std::find_if_not(++valueIter, fieldValue.end(), isgraph);
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
	if (this->headerFields.find("host") == this->headerFields.end())
		throw(ResponseException(400, "Missing Host header field"));
}

//host header field needs additional checks
// 1/ host from header needs to be parsed and percent-encoding has to be resolved (the latter is not specified by RFC but it's a nice-to-have feature)
// 2/ that does not need ot be done if there already is a host under requestLine.authority - but it should probably still be checked for invalid characters and separators
// 3/ identify comma-separated hosts (or automatically consider them as a singleton field)
stringpair_t	HttpRequest::resolveHost(void)
{
	stringmap_t::iterator	hostIter = this->headerFields.find("host");
	stringpair_t			hostHeader = this->parseAuthority(hostIter->second, HEADER_FIELD);
	stringpair_t			authority("","");
	size_t					pos = 0;
	pos = hostHeader.first.find('%');
	while (pos != std::string::npos && pos < hostHeader.first.size())
	{
		resolvePercentEncoding(hostHeader.first, pos);
		pos = hostHeader.first.find('%', pos);
	}

	if (this->requestLine.requestTarget.authority.first != "")
	{
		authority.first = this->requestLine.requestTarget.authority.first;
		authority.second = this->requestLine.requestTarget.authority.second;
	}
	else if (hostHeader.first != "")
	{
		authority.first = hostHeader.first;
		authority.second = hostHeader.second;
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
		/*
		A sender MUST NOT send whitespace between the start-line and the first header field.
		A recipient that receives whitespace between the start-line and the first header field
		MUST either reject the message as invalid ...
		*/
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
	//std::cout << this->requestLine << this->headerFields << std::endl;
	std::cout << "Final host:\t" << authority.first << std::endl;
	std::cout << "Final port:\t" << authority.second << std::endl;
	return (authority);
}

// has to be present before anything that can keep a persistent connection,
// e.g. Expect, 3xx responses
// handling contradicting values is implementation-specific; we just search for "close", otherwise the connection is kept alive - should be done the other way around for 1.0
// also checks for Keep-Alive header if close isn't specified
void	HttpRequest::validateConnectionOption(void)
{
	std::string							allowedChars(std::string(DIGITS) + std::string(ALPHA) + std::string(TOKEN) + "=");
	stringmap_t::iterator				connection = this->headerFields.find("connection");
	stringmap_t::iterator				keepAlive = this->headerFields.find("keep-alive");
	std::vector<std::string>			values;
	std::vector<std::string>::iterator	option;
	std::vector<std::string>::iterator	param;
	std::vector<std::string>			paramValues;
	if (connection != this->headerFields.end())
	{
		values = splitQuotedString(connection->second, ',');
		for (option = values.begin(); option != values.end(); option++)
		{
			*option = trim(*option);
			if (option->find_first_not_of(allowedChars) != std::string::npos)
				throw (ResponseException(400, "Invalid characters in Connection header field"));
			std::transform(option->begin(), option->end(), option->begin(), tolower);
		}
		if (std::find(values.begin(), values.end(), "close") != values.end())
			connectionStatus = CLOSE;
		else
			connectionStatus = KEEP_ALIVE;
	}
	else
		this->connectionStatus = KEEP_ALIVE;
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
			paramValues = splitQuotedString(*param, '=');
			if (paramValues.size() == 2 && paramValues[0] == "max")
				std::cout << "MAX REQUESTS/CLIENT PLACEHOLDER: " << paramValues[1] << std::endl;
			else if (paramValues.size() == 2 && paramValues[0] == "timeout")
				std::cout << "MAX TIMEOUT/CLIENT PLACEHOLDER: " << paramValues[1] << std::endl;
		}
	}
}

// http://hello//testdir/a will get redirected to /testdir/a/
void	HttpRequest::validateResourceAccess(const Location& location)
{
	this->allowedDirListing = location.getAutoindex();
	std::cout << location.getPath() << std::endl;
	if (*root.rbegin() == '/')
		root.erase(root.end() - 1);
	this->targetResource = root + this->requestLine.requestTarget.absolutePath;
	std::cout << CLR3 << "Final path:\t" << this->targetResource << RESET << std::endl;
	
	if (!this->isRedirect && this->requestLine.method == "GET")
	{
		int			validFile;
		struct stat	fileCheckBuff;
		validFile = stat(targetResource.c_str(), &fileCheckBuff);
		if (validFile < 0)
			throw (ResponseException(404, "File does not exist"));
		if (*targetResource.rbegin() != '/') // if it's a normal file, do nothing
		{
			if (fileCheckBuff.st_mode & S_IFDIR)
				throw (ResponseException(301, "")); // will likely not be an exception, but a proper response handler
		}
		else if (*targetResource.rbegin() == '/' && !this->allowedDirListing)
			throw (ResponseException(403, "Autoindexing is off"));
		else
		{
			// autoindexing for GET - should go into response && throw internal server error if applicable
			DIR*	dirPtr;
			dirPtr = opendir(this->targetResource.c_str());
			if (dirPtr == NULL)
				throw (ResponseException(500, "Failed to open directory"));
			for (dirent* dir = readdir(dirPtr); dir != NULL; dir = readdir(dirPtr))
				std::cout << CLR2 << dir->d_name << RESET << std::endl;
			if (closedir(dirPtr))
				throw (ResponseException(500, "Failed to close directory"));
		}
	}
	// handle redirections, POST and DELETE
}

void	HttpRequest::validateMessageFraming(void)
{
	stringmap_t::iterator	transferEncoding = this->headerFields.find("transfer-encoding");
	stringmap_t::iterator	contentLength = this->headerFields.find("content-length");
	std::vector<std::string>	values;
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
			throw (ResponseException(417, ""));
		else
			throw (ResponseException(100, "")); // should probably be done differently
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
	std::set<std::string>	allowedMethods = location.getAllowMethods();
	this->allowedMethods = allowedMethods;
	if (std::find(allowedMethods.begin(), allowedMethods.end(), this->requestLine.method) == allowedMethods.end())
		throw (ResponseException(405, ""));
	this->validateConnectionOption();
	this->validateResourceAccess(location);
	this->validateMessageFraming();
	this->manageExpectations();
}

// check max body size
// also should check for valid characters  in extension(?)
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
		this->requestBody = octets_t(bufferedBody.begin(), bufferedBody.begin() + this->contentLength);
		this->readingBodyInProgress = false;
		this->requestComplete = true;
		return (this->requestBody.size());
	}
	else if (this->messageFraming == TRANSFER_ENCODING)
	{
		std::cout << CLR2 << "Received: " << bufferedBody.size() << " bytes" << RESET << std::endl;
		for (octets_t::iterator	it = bufferedBody.begin() ; it != bufferedBody.end() ; it = bufferedBody.begin())
		{
			octets_t::iterator	newline = std::find(bufferedBody.begin(), bufferedBody.end(), '\n');
			octets_t			chunkSizeLine(bufferedBody.begin(), newline);
			size_t				tempBytesRead = chunkSizeLine.size() + 1;
			octets_t::iterator	semicolon = std::find(chunkSizeLine.begin(), chunkSizeLine.end(), ';');
			octets_t			chunkSizeOct(chunkSizeLine.begin(), semicolon); // ignore chunk-extension
			std::cout << "Chunk size: " << chunkSizeOct << std::endl;
			bufferedBody.erase(bufferedBody.begin(), newline + 1); // move to the beginning of the chunk
			if (chunkSizeOct.size() > 0 && !isxdigit(chunkSizeOct[0]))
				throw (ResponseException(400, "Invalid chunk size value (whitespace)")); // anything else than a hexdigit at the start isn't allowed
			if (std::count(chunkSizeOct.begin(), chunkSizeOct.end(), '\0') > 0)
				throw (ResponseException(400, "Invalid chunk size value (null byte)")); // needs special check prior to conversion to string
			std::string		chunkSizeStr(chunkSizeOct.begin(), chunkSizeOct.end());
			chunkSizeStr = trim(chunkSizeStr);
			if (chunkSizeStr.find_first_not_of(HEXDIGITS) != std::string::npos)
				throw (ResponseException(400, "Invalid chunk size value"));
			errno = 0;
			size_t chunkSize = strtoul(chunkSizeStr.c_str(), NULL, 16);
			int	error = errno;
			if (error == ERANGE || chunkSize > INT_MAX) // update to max_body_size too
				throw (ResponseException(413, "Chunk is too big"));

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

const octets_t &HttpRequest::getRequestBody(void) const
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
	this->targetResource.clear();
	this->connectionStatus = newRequest.connectionStatus;
	this->allowedDirListing = newRequest.allowedDirListing;
	this->isRedirect = newRequest.isRedirect;
	this->contentLength = newRequest.contentLength;
	this->messageFraming = newRequest.messageFraming;
	this->requestComplete = newRequest.requestComplete;
	this->readingBodyInProgress = newRequest.readingBodyInProgress;
	this->response = newResponse;
}