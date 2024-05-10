/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHeader.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/09 09:56:07 by plouda            #+#    #+#             */
/*   Updated: 2024/05/10 10:09:18 by plouda           ###   ########.fr       */
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
	std::vector<octets_t>			splitTokens;
	std::vector<uint8_t>::iterator	space = std::find(startLine.begin(), startLine.end(), SP);


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

// needs to check for bare CR
// also needs to check for no SP between first line and first field
void	HttpHeader::parseHeader(octets_t header)
{
	std::vector<octets_t>			splitLines;
	octets_t::iterator				nl = std::find(header.begin(), header.end(), '\n');
	bool	startLine = true;

	while (nl != header.end())
	{
		octets_t line(header.begin(), nl);
		if ((line.size() == 1 && line[0] == '\r') || line.size() == 0) // indicates the end of field section
			break ;
		if (startLine)
			this->parseStartLine(line);
		if (!startLine)
			splitLines.push_back(line);
		header.erase(header.begin(), nl + 1);
		nl = std::find(header.begin(), header.end(), '\n');
		startLine = false;
	}
	std::cout << splitLines << std::flush;
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

