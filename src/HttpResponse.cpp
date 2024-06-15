/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 10:52:29 by plouda            #+#    #+#             */
/*   Updated: 2024/06/15 08:54:00 by plouda           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/HttpResponse.hpp"
#include "../inc/HttpRequest.hpp"

HttpResponse::HttpResponse()
{
	this->statusLine.httpVersion = "HTTP/1.1";
	this->statusLine.statusCode = 200;
	this->statusLine.reasonPhrase = "OK";
	this->headerFields.insert(std::make_pair("Server:", "webserv/nginx-but-better"));
	return ;
}

// Sun, 06 Nov 1994 08:49:37 GMT 
std::string	getIMFFixdate(void)
{
	time_t		curr_time;
	tm			*curr_tm;
	char		buffer[100];

	std::time(&curr_time);
	curr_tm = std::gmtime(&curr_time);
	std::strftime(buffer, 100, "%a, %d %b %Y %H:%M:%S GMT", curr_tm);
	return (std::string(buffer));
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

// needs different Content-Type
// needs proper allowed methods handling
void	HttpResponse::prepareResponseHeaders(const HttpRequest& request)
{
	size_t		contentLength = this->responseBody.size();
	std::string	date = getIMFFixdate();
	std::string	type("text/html");
	this->headerFields.insert(std::make_pair("Content-Length:", itoa(contentLength)));
	this->headerFields.insert(std::make_pair("Date:", date));
	this->headerFields.insert(std::make_pair("Content-Type:", type));
	if (request.getConnectionStatus() == CLOSE)
		this->headerFields.insert(std::make_pair("Connection:", "close"));
	else
	{
		this->headerFields.insert(std::make_pair("Connection:", "keep-alive"));
		this->headerFields.insert(std::make_pair("Keep-Alive:", std::string("timeout=" + itoa(CONNECTION_TIMEOUT))));
	}
	if (this->statusLine.statusCode == 405)
	{
		std::string	methods;
		std::set<std::string>::const_iterator it = request.getAllowedMethods().begin();
		while (it != request.getAllowedMethods().end())
		{
			methods.append(*it);
			if (++it != request.getAllowedMethods().end())
				methods.append(", ");
		}
		this->headerFields.insert(std::make_pair("Allow:", methods));
	}
	
}

HttpResponse::ResponseException::ResponseException() : std::invalid_argument("")
{
	return ;
}

const char *HttpResponse::ResponseException::what() const throw()
{
	return ("Response exception thrown, proper response sent to the client\n");
}
