/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseException.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/18 14:10:47 by plouda            #+#    #+#             */
/*   Updated: 2024/09/02 13:47:24 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSEEXCEPTION_HPP
#define RESPONSEEXCEPTION_HPP

#include "webserv.hpp"
#include "HttpRequest.hpp"

class ResponseException : public std::invalid_argument
{
	private:
		statusLine_t	_statusLine;
		std::string		_details;
		ResponseException();
	public:
		ResponseException(unsigned short status, std::string details);
		const char*		what() const throw();
		virtual ~ResponseException() throw() {};
		
		const statusLine_t&	getStatusLine() const;
		const std::string&	getStatusDetails() const;
};

#endif  // RESPONSEEXCEPTION_HPP
