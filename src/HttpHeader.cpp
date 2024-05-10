/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHeader.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/09 09:56:07 by plouda            #+#    #+#             */
/*   Updated: 2024/05/10 15:38:13 by plouda           ###   ########.fr       */
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

void	HttpHeader::parseStartLine(octets_t startLine)
{
	std::vector<octets_t>	splitTokens;
	octets_t::iterator		space = std::find(startLine.begin(), startLine.end(), SP);


	while (space != startLine.end() || startLine.size() != 0)
	{	
		octets_t token(startLine.begin(), space);
		splitTokens.push_back(token);
		while (isspace(*space))
			space++;
		startLine.erase(startLine.begin(), space);
		space = std::find(startLine.begin(), startLine.end(), SP);
	}
	std::cout << splitTokens << std::flush;
}

/* "In the interest of robustness, a server that is expecting to receive and parse a request-line
 SHOULD ignore at least one empty line (CRLF) received prior to the request-line." */
 void	trimHeaderEmptyLines(octets_t& header)
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
		throw (std::invalid_argument("400 Bad request: Bare CR detected"));
	else if (countCR == 1)
	{
		octets_t::iterator	cr = std::find(line.begin(), line.end(), '\r');
		if ((cr + 1) != line.end())
			throw (std::invalid_argument("400 Bad request: Bare CR detected"));
	}
}

void	HttpHeader::parseHeader(octets_t header)
{
	try
	{
		trimHeaderEmptyLines(header);
		std::vector<octets_t>			splitLines;
		bool							startLine = true;
		bool							endLine = false;
		octets_t::iterator				nl = std::find(header.begin(), header.end(), '\n');
		octets_t 						line;
		while (nl != header.end())
		{
			octets_t line(header.begin(), nl);
			invalidateBareCR(line);
			if (line.size() == 0 || (line.size() == 1 && line[0] == '\r')) // indicates the end of field section
			{
				endLine = true;
				break ;
			}
			if (startLine)
				this->parseStartLine(line);
			if (!startLine)
				splitLines.push_back(line);
			header.erase(header.begin(), nl + 1);
			nl = std::find(header.begin(), header.end(), '\n');
			/*
			A sender MUST NOT send whitespace between the start-line and the first header field.
			A recipient that receives whitespace between the start-line and the first header field
			MUST either reject the message as invalid 
			*/
			if (startLine && isspace(*header.begin()))
				throw (std::invalid_argument("400 Bad request: Dangerous whitespace detected"));
			startLine = false;
		}
		if (startLine)
			throw (std::invalid_argument("400 Bad request: Empty"));
		if (nl == header.end() && !endLine) // the loop should only exit if there's a valid CRLF
			throw (std::invalid_argument("400 Bad request: Missing empty CRLF"));
		if (std::distance(header.begin(), nl) == 1 && *header.begin() != '\r')
			throw (std::invalid_argument("400 Bad request: Improperly terminated header-field section"));

		std::cout << splitLines << std::flush;
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

