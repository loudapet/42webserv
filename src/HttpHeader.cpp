/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHeader.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/09 09:56:07 by plouda            #+#    #+#             */
/*   Updated: 2024/05/15 12:03:04 by plouda           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/HttpHeader.hpp"

HttpHeader::HttpHeader()
{
	return ;
}

HttpHeader::HttpHeader(const HttpHeader& refObj)
{
	*this = refObj;
}

HttpHeader& HttpHeader::operator = (const HttpHeader& refObj)
{
	(void)refObj;
	return (*this);
}

HttpHeader::~HttpHeader()
{
	return ;
}

void	HttpHeader::parseMethod(std::string& token)
{
	if (token == "GET" || token == "POST" || token == "DELETE")
		this->startLine.method = token;
	else
		throw(std::invalid_argument("400 Bad Request: Invalid method")); // should probably be Not implemented
}

static void resolvePercentEncoding(std::string& absolutePath, size_t pos)
{
	if (absolutePath[pos] == '%')
		{
			if (pos + 2 < absolutePath.size())
			{	
				std::string	hexDigit = absolutePath.substr(pos + 1, 2);
				std::cout << "hexdigit: " << hexDigit << std::endl;
				if (hexDigit.find_first_not_of(HEXDIGITS) != std::string::npos)
					throw(std::invalid_argument("400 Bad Request: Invalid percent encoding"));
				int	c = strtol(hexDigit.c_str(), NULL, 16);
				std::cout << c << std::endl;
				if (c >= 128)
					throw(std::invalid_argument("400 Bad Request: Invalid percent encoding - out of range"));
				if (isalnum(c) || std::strchr("-._~:@", c) != NULL || std::strchr(SUBDELIMS, c) != NULL)
				{
					absolutePath.erase(pos, 3);
					absolutePath.insert(pos, 1, static_cast<char>(c));
				}
			}
			else					
				throw(std::invalid_argument("400 Bad Request: Invalid percent encoding"));
		}
}

void	HttpHeader::validatePath(std::string& absolutePath)
{
	const std::string	allowedChars(std::string(UNRESERVED) + std::string(PCHAR_EXTRA));
	size_t pos = absolutePath.find_first_not_of(allowedChars);
	if (pos != std::string::npos)
	{
		resolvePercentEncoding(absolutePath, pos);
	}
}

void	HttpHeader::parseRequestTarget(std::string& uri)
{
	if (uri.find_first_of('/') != 0 || uri.find_first_of('#') != std::string::npos)
		throw(std::invalid_argument("400 Bad Request: Invalid URI path"));
	std::string::iterator	endPos = std::find(uri.begin(), uri.end(), '?');
	if (endPos != uri.end())
	{
		this->startLine.requestTarget.absolutePath = std::string(uri.begin(), endPos);
		this->startLine.requestTarget.query = std::string(endPos, uri.end());
	}
	else
	{
		this->startLine.requestTarget.absolutePath = std::string(uri);
		this->startLine.requestTarget.query = std::string("");
	}
	validatePath(this->startLine.requestTarget.absolutePath);
	return ;
}

// pending response to invalid versions and to version 1.0
void	HttpHeader::parseHttpVersion(std::string& token)
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

void	HttpHeader::parseStartLine(std::string startLine)
{
	std::vector<std::string>	startLineTokens;
	std::string::iterator		space = std::find(startLine.begin(), startLine.end(), SP);
	ParseToken					parse[3] = {&HttpHeader::parseMethod,
								&HttpHeader::parseRequestTarget,
								&HttpHeader::parseHttpVersion};

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
	std::cout << this->startLine << std::endl;
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

void	HttpHeader::parseHeader(octets_t header)
{
	try
	{
		trimHeaderEmptyLines(header);
		std::vector<std::string>		splitLines;
		bool							startLine = true;
		bool							endLine = false;
		octets_t::iterator				nl = std::find(header.begin(), header.end(), '\n');
		octets_t 						line;
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
		std::cout << splitLines << std::endl;
	}
	catch(const std::exception& e)
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
	os << "method: " << startLine.method << std::endl;
	os << "request-target path: " << startLine.requestTarget.absolutePath << std::endl;
	os << "request-target query: " << startLine.requestTarget.query << std::endl;
	os << "http-version: " << startLine.httpVersion << std::endl;
	return os;
}

