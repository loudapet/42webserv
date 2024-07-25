/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseException.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/18 14:11:51 by plouda            #+#    #+#             */
/*   Updated: 2024/07/22 17:36:56 by plouda           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ResponseException.hpp"

ResponseException::ResponseException() : std::invalid_argument("")
{
	return ;
}

ResponseException::ResponseException(unsigned short status, std::string details) : std::invalid_argument("")
{
	this->_statusLine.httpVersion = "HTTP/1.1";
	this->_statusLine.statusCode = status;
	this->_statusLine.reasonPhrase = "";
	this->_details = details;
	//Logger::safeLog(INFO, RESPONSE, itoa(this->_statusLine.statusCode) + " ", this->_details);
}

const char*	ResponseException::what() const throw()
{
	return ("Response exception thrown, proper response sent to the client\n");
}

const statusLine_t&	ResponseException::getStatusLine() const
{
	return (this->_statusLine);
}

const std::string&	ResponseException::getStatusDetails() const
{
	return (this->_details);
}
