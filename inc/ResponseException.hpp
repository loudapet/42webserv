/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseException.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/18 14:10:47 by plouda            #+#    #+#             */
/*   Updated: 2024/06/21 10:30:44 by plouda           ###   ########.fr       */
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
		virtual ~ResponseException() throw() {}; // no idea why, but it won't compile otherwise
		
		const statusLine_t&	getStatusLine() const;
		const std::string&	getStatusDetails() const;
};

#endif  // RESPONSEEXCEPTION_HPP
