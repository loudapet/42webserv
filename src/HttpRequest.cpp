/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/09 09:56:07 by plouda            #+#    #+#             */
/*   Updated: 2024/06/04 10:56:50 by plouda           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/HttpRequest.hpp"

HttpRequest::HttpRequest()
{
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

void	HttpRequest::parseMethod(std::string& token)
{
	if (token == "GET" || token == "POST" || token == "DELETE")
		this->startLine.method = token;
	else
		throw(std::invalid_argument("400 Bad Request: Invalid method")); // should probably be Not implemented
}

void resolvePercentEncoding(std::string& path, size_t& pos)
{
	if (path[pos] == '%')
	{
		if (pos + 2 < path.size())
		{	
			std::string	hexDigit = path.substr(pos + 1, 2);
			if (hexDigit.find_first_not_of(HEXDIGITS) != std::string::npos)
				throw(std::invalid_argument("400 Bad Request: Invalid percent encoding"));
			int	c = strtol(hexDigit.c_str(), NULL, 16);
			if (c)
			{
				path.erase(pos, 3);
				path.insert(pos, 1, static_cast<char>(c));
			}
			else
				throw(std::invalid_argument("400 Bad Request: Zero byte insertion detected"));
			pos++;
		}
		else					
			throw(std::invalid_argument("400 Bad Request: Invalid percent encoding"));
	}
}

void	HttpRequest::resolveDotSegments(std::string& path)
{
	std::stack<std::string>	segments;
	std::stack<std::string>	output;
	std::stringstream	input(path);
	std::string			buffer;
	std::string			newPath;
	output.push("/");
	while (std::getline(input, buffer, '/'))
	{
		if (buffer == "..")
		{
			if (output.size() > 1)
				output.pop();
			else
				throw(std::invalid_argument("400 Bad Request: Invalid relative reference"));
		}
		else if (buffer != "." && buffer != "")
			output.push(buffer + std::string("/"));
	}
	if (*(output.top().rbegin()) == '/' && *(path.rbegin()) != '/' && output.top().size() > 0)
		output.top().erase(output.top().size() - 1, 1);
	while (output.size() > 0)
	{
		newPath.insert(0, output.top());
		output.pop();
	}
	this->startLine.requestTarget.absolutePath = newPath;
	return ;
}

void	HttpRequest::validateURIElements(void)
{
	size_t			pos = 0;
	std::string		extraAllowedChars("?#\0\0", 4);
	std::string		allowedChars(std::string(UNRESERVED) + std::string(PCHAR_EXTRA)
									+ std::string(SUBDELIMS));
	std::string*	elements[] = 	{&this->startLine.requestTarget.absolutePath,
									&this->startLine.requestTarget.query,
									&this->startLine.requestTarget.fragment,
									&this->startLine.requestTarget.authority.first};
	for (size_t i = 0; i < 4; i++)
	{
		pos = (*(elements)[i]).find_first_not_of(allowedChars);
		while (pos != std::string::npos && pos < (*(elements)[i]).size())
		{
			if ((*(elements)[i])[pos] == '%')
				resolvePercentEncoding(*(elements)[i], pos);
			else
				throw(std::invalid_argument("400 Bad Request: Invalid character in URI"));
			pos = (*(elements)[i]).find_first_not_of(allowedChars, pos);
		}
		allowedChars.push_back(extraAllowedChars[i]);
	}
}

stringpair_t	HttpRequest::parseAuthority(std::string& authority, HostLocation parseLocation)
{
	if (authority.find_first_of("@") != std::string::npos)
		throw(std::invalid_argument("400 Bad Request: 'userinfo' component deprecated"));
	std::string		allowedChars(std::string(UNRESERVED) + std::string(SUBDELIMS) + std::string("%"));
	std::string		host("");
	std::string		port("");
	size_t			portPos = authority.find_first_of(":");
	if (portPos == std::string::npos)
	{
		host = std::string(authority);
		port = DEFAULT_PORT;
	}
	else if (portPos == 0)
	{
		if (parseLocation == URI)
			throw(std::invalid_argument("400 Bad Request: Empty host name in URI"));
		else
			throw(std::invalid_argument("400 Bad Request: Empty host name in host header field"));
	}
	else
	{
		host = std::string(authority.begin(), authority.begin() + portPos);
		port = std::string(authority.begin() + portPos + 1, authority.end());
		if (port == "")
			port = DEFAULT_PORT;
	}
	if (host.find_first_not_of(allowedChars) != std::string::npos)
	{
		if (parseLocation == URI)
			throw(std::invalid_argument("400 Bad Request: Invalid host name in URI"));
		else
			throw(std::invalid_argument("400 Bad Request: Invalid host name in host header field"));
	}
	if (port.find_first_not_of(DIGITS) != std::string::npos)
	{
		if (parseLocation == URI)
			throw(std::invalid_argument("400 Bad Request: Invalid port format in URI"));
		else
			throw(std::invalid_argument("400 Bad Request: Invalid port format in host header field"));
	}
	std::transform(host.begin(), host.end(), host.begin(), tolower); // case-insensitive
	return(std::make_pair(host, port));
}

void	HttpRequest::parseRequestTarget(std::string& uri)
{
	//the following two lines should be in the constructor
	this->startLine.requestTarget.authority.first = "";
	this->startLine.requestTarget.authority.second = "";
	if (uri.find_first_of('/') != 0)
	{
		std::string scheme(uri.begin(), uri.begin() + 7);
		std::transform(scheme.begin(), scheme.end(), scheme.begin(), tolower);
		if (scheme.find("http://") != 0)
			throw(std::invalid_argument("400 Bad Request: Invalid URI path"));
		else
		{
			std::string	delimiters("?#/");
			uri.erase(uri.begin(), uri.begin() + 7); // remove "http://"
			if (uri.size() == 0 || uri.find_first_of(delimiters) == 0)
				throw(std::invalid_argument("400 Bad Request: Authority required"));
			else
			{
				std::string	authority = std::string(uri.begin(), std::find_first_of(uri.begin(), uri.end(), delimiters.begin(), delimiters.end()));
				if (authority.size() == 0)
					throw(std::invalid_argument("400 Bad Request: Authority required"));
				this->startLine.requestTarget.authority = this->parseAuthority(authority, URI);
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
		this->startLine.requestTarget.absolutePath = std::string(uri.begin(), uri.end());
		this->startLine.requestTarget.query = std::string("");
		this->startLine.requestTarget.fragment = std::string("");
	}
	else if (queryPos != uri.end() && fragmentPos != uri.end())
	{
		this->startLine.requestTarget.absolutePath = std::string(uri.begin(), queryPos);
		this->startLine.requestTarget.query = std::string(queryPos, fragmentPos);
		this->startLine.requestTarget.fragment = std::string(fragmentPos, uri.end());
	}
	else if (queryPos != uri.end() && fragmentPos == uri.end())
	{
		this->startLine.requestTarget.absolutePath = std::string(uri.begin(), queryPos);
		this->startLine.requestTarget.query = std::string(queryPos, uri.end());
		this->startLine.requestTarget.fragment = std::string("");
	}
	else if (queryPos == uri.end() && fragmentPos != uri.end())
	{
		this->startLine.requestTarget.absolutePath = std::string(uri.begin(), fragmentPos);
		this->startLine.requestTarget.query = std::string("");
		this->startLine.requestTarget.fragment = std::string(fragmentPos, uri.end());
	}
	if (this->startLine.requestTarget.absolutePath == "")
		this->startLine.requestTarget.absolutePath = "/";
	validateURIElements();
	resolveDotSegments(this->startLine.requestTarget.absolutePath);
	return ;
}

// pending response to invalid versions and to version 1.0
void	HttpRequest::parseHttpVersion(std::string& token)
{
	std::string::iterator	slash = std::find(token.begin(), token.end(), '/');
	std::string	http(token.begin(), slash);
	if (http != "HTTP" || slash == token.end() || slash + 1 == token.end())
		throw(std::invalid_argument("400 Bad Request: Invalid protocol specification"));
	std::string	version(slash + 1, std::find_if(token.begin(), token.end(), isspace));
	if (version != "1.1")
		throw(std::invalid_argument("505 Version Not Supported"));
	this->startLine.httpVersion = token;
}

void	HttpRequest::parseStartLine(std::string startLine)
{
	std::vector<std::string>	startLineTokens;
	std::string::iterator		space = std::find(startLine.begin(), startLine.end(), SP);
	ParseToken					parse[3] = {&HttpRequest::parseMethod,
									&HttpRequest::parseRequestTarget,
									&HttpRequest::parseHttpVersion};

	while (space != startLine.end() || startLine.size() != 0) // size != 0 to grab the last segment
	{
		std::string token(startLine.begin(), space);
		startLineTokens.push_back(token);
		while (space != startLine.end() && isspace(*space))
			space++;
		startLine.erase(startLine.begin(), space);
		space = std::find(startLine.begin(), startLine.end(), SP);
	}
	if (startLineTokens.size() != 3)
		throw(std::invalid_argument("400 Bad Request: Invalid start line"));
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
			throw(std::invalid_argument("400 Bad Request: Invalid field name"));
		fieldValue = std::string(++fieldIter, it->end()); // to remove ':' from the value part
		fieldValue = trim(fieldValue);
		valueIter = std::find_if_not(fieldValue.begin(), fieldValue.end(), isgraph);
		while (valueIter != fieldValue.end())
		{
			/* std::bitset<8> x(*valueIter);
			std::cout << x << std::endl; */
			if (*valueIter >> 7) // UTF-8 encoding starts with 1 in its most-significant byte
			{
				valueIter = std::find_if_not(++valueIter, fieldValue.end(), isgraph);
				continue;
			}
			if (!isblank(*valueIter))
				throw(std::invalid_argument("400 Bad Request: Invalid field value - CTL characters forbidden"));
			else if (isblank(*valueIter) && isblank(*(valueIter + 1)))
				throw(std::invalid_argument("400 Bad Request: Invalid field value - multiple SP / HTAB detected"));
			valueIter = std::find_if_not(++valueIter, fieldValue.end(), isgraph);
		}
		if (!(this->headerFields.insert(std::make_pair(fieldName, fieldValue)).second)) // handle insertion of duplicates by concatenation
		{
			if (fieldName == "host")
				throw(std::invalid_argument("400 Bad Request: Duplicate Host header field"));
			mapIter = this->headerFields.find(fieldName);
			mapIter->second.append(std::string(",") + std::string(fieldValue));
		}
	}
	if (this->headerFields.find("host") == this->headerFields.end())
		throw(std::invalid_argument("400 Bad Request: Missing Host header field"));
}

//host header field needs additional checks
// 1/ host from header needs to be parsed and percent-encoding has to be resolved
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

	if (this->startLine.requestTarget.authority.first != "")
	{
		authority.first = this->startLine.requestTarget.authority.first;
		authority.second = this->startLine.requestTarget.authority.second;
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
		throw (std::invalid_argument("400 Bad Request: Bare CR detected"));
	else if (countCR == 1)
	{
		octets_t::iterator	cr = std::find(line.begin(), line.end(), '\r');
		if ((cr + 1) != line.end())
			throw (std::invalid_argument("400 Bad Request: Bare CR detected"));
	}
}

static void	invalidateNullBytes(octets_t& line)
{
	size_t	countNull = std::count(line.begin(), line.end(), '\0');
	if (countNull)
		throw (std::invalid_argument("400 Bad Request: Zero bytes disallowed"));
}

stringpair_t	HttpRequest::parseHeader(octets_t header)
{
	try
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
				this->parseStartLine(std::string(line.begin(), line.end()));
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
				throw (std::invalid_argument("400 Bad Request: Dangerous whitespace detected"));
			startLine = false;
		}
		if (startLine)
			throw (std::invalid_argument("400 Bad Request: Empty start line"));
		if (nl == header.end() && !endLine) // the loop should only exit if there's a valid CRLF
			throw (std::invalid_argument("400 Bad Request: Missing empty CRLF"));
		if (std::distance(header.begin(), nl) == 1 && *header.begin() != '\r')
			throw (std::invalid_argument("400 Bad Request: Improperly terminated header-field section"));
		this->parseFieldSection(splitLines);
		authority = this->resolveHost();
		std::cout << this->startLine << this->headerFields << std::endl;
		std::cout << "Final host:\t" << authority.first << std::endl;
		std::cout << "Final port:\t" << authority.second << std::endl;
		return (authority);
	}
	catch(const std::invalid_argument& e)
	{
		std::cerr << e.what() << '\n';
		return (std::make_pair(std::string(""), std::string(DEFAULT_PORT)));
	}
}

/*
1) method check based on string comparison once the server block rules are matched, also needs 405 Method Not Allowed if applicable
2) 
*/
void	HttpRequest::validateHeader(void)
{
	try
	{
		if (this->startLine.method != "GET" && this->startLine.method != "POST"
		&& this->startLine.method != "DELETE")
			throw (std::invalid_argument("501 Not implemented"));
	}
	catch(const std::invalid_argument& e)
	{
		std::cerr << e.what() << '\n';
	}

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

std::ostream &operator<<(std::ostream &os, startLine_t& startLine)
{
	os << UNDERLINE << "Start line" << RESET << std::endl;
	os << "method: " << startLine.method << std::endl;
	os << "request-target host: " << startLine.requestTarget.authority.first << std::endl;
	os << "request-target port: " << startLine.requestTarget.authority.second << std::endl;
	os << "request-target path: " << startLine.requestTarget.absolutePath << std::endl;
	os << "request-target query: " << startLine.requestTarget.query << std::endl;
	os << "request-target fragment: " << startLine.requestTarget.fragment << std::endl;
	os << "http-version: " << startLine.httpVersion << std::endl;
	return os;
}

std::ostream &operator<<(std::ostream &os, std::map<std::string, std::string>& fieldSection)
{
	os << UNDERLINE << "Header fields" << RESET << std::endl;
	for (std::map<std::string, std::string>::iterator it = fieldSection.begin(); it != fieldSection.end(); it++)
		os << it->first << ": " << it->second << std::endl;
	return os;
}

