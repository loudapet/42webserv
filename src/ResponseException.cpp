/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseException.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/18 14:11:51 by plouda            #+#    #+#             */
/*   Updated: 2024/09/02 14:59:59 by aulicna          ###   ########.fr       */
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
