/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 10:48:46 by plouda            #+#    #+#             */
/*   Updated: 2024/06/14 16:44:11 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP
#include <iostream>
#include <ctime>
#include "webserv.hpp"
#include "Location.hpp"

typedef struct StatusLine
{
	std::string 	httpVersion; // "HTTP/" DIGIT "." DIGIT
	unsigned short	statusCode; // 2xx, 3xx, ...
	std::string		reasonPhrase; // Bad Request, Entity Too Large, ...
} statusLine_t;

class HttpResponse
{
	private:
		statusLine_t	statusLine;
		stringmap_t		headerFields;
		std::string		responseBody;
		octets_t		completeResponse;

	public:
		HttpResponse();
		HttpResponse(const HttpResponse& refObj);
		HttpResponse& operator = (const HttpResponse& refObj);
		~HttpResponse();

		const statusLine_t&	getStatusLine()	const;
		void				throwResponseException(unsigned short status, std::string reason, std::string details);
		void				prepareResponseHeaders();
		std::string			readErrorPage(const Location &location);

		class ResponseException : public std::invalid_argument
		{
			public:
				ResponseException();
				const char*		what() const throw();
				
		};
};

#endif  // HTTPRESPONSE_HPP
