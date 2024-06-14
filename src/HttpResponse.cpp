/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 10:52:29 by plouda            #+#    #+#             */
/*   Updated: 2024/06/14 16:48:23 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"

HttpResponse::HttpResponse()
{
	this->statusLine.httpVersion = "HTTP/1.1";
	this->statusLine.statusCode = 200;
	this->statusLine.reasonPhrase = "OK";
	this->headerFields.insert(std::make_pair("Server:", "webserv auplok"));
	return ;
}

// Sun, 06 Nov 1994 08:49:37 GMT 
std::string	getIMFFixdate(void)
{
	time_t		curr_time;
	tm			*curr_tm;
	char		buffer[100];

	std::time(&curr_time);
	curr_tm = std::localtime(&curr_time);
	std::cout << "[";
	std::strftime(buffer, 100, "%Y%m%d_%H%M%S", curr_tm);
	std::cout << buffer;
	//std::cout << "19920104_091532";
	std::cout << "] ";

}

HttpResponse::HttpResponse(const HttpResponse& refObj)
{
	*this = refObj;
}

HttpResponse& HttpResponse::operator = (const HttpResponse& refObj)
{
	if (this != &refObj)
		return (*this);
	return (*this);
}

HttpResponse::~HttpResponse()
{
	return ;
}

const statusLine_t	&HttpResponse::getStatusLine() const
{
	return (this->statusLine);
}

// maybe status as a string?
void	HttpResponse::throwResponseException(unsigned short status, std::string reason, std::string details)
{
	this->statusLine.statusCode = status;
	this->statusLine.reasonPhrase = reason;
	this->responseBody.append(details);
	throw (ResponseException());
}

void	HttpResponse::prepareResponseHeaders(void)
{
	size_t	contentLength = this->responseBody.size();
	
}

HttpResponse::ResponseException::ResponseException() : std::invalid_argument("")
{
	return ;
}

const char *HttpResponse::ResponseException::what() const throw()
{
	return ("Response exception thrown, proper response sent to the client\n");
}

std::string	HttpResponse::readErrorPage(const Location &location)
{
	const std::map<unsigned short, std::string> &errorPages = location.getErrorPages();
	std::ifstream											errorPageFile;
	std::stringstream										buff;
	std::map<unsigned short, std::string>::const_iterator	it;
	std::stringstream										ss;
	
	it = errorPages.find(this->statusLine.statusCode);
	if (it != errorPages.end())
	{
		errorPageFile.open(it->second.c_str());
		if (errorPageFile)
		{
			buff << errorPageFile.rdbuf();
			return (buff.str());
		}
	}
	ss << "<html>\r\n"
	   << "<head><title>" << this->statusLine.statusCode << " " << this->statusLine.reasonPhrase << "</title></head>\n"
	   << "<body>\r\n"
	   << "<center><h1>" << this->statusLine.statusCode << this->statusLine.reasonPhrase << "</h1></center>\r\n"
	   << "</html>\r\n";
	return (ss.str());
}
