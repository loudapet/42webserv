/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 10:52:29 by plouda            #+#    #+#             */
/*   Updated: 2024/06/14 14:50:57 by plouda           ###   ########.fr       */
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
