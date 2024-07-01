/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 10:48:46 by plouda            #+#    #+#             */
/*   Updated: 2024/06/28 15:22:09 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP
#include <iostream>
#include <iomanip>
#include <ctime>
#include "Location.hpp"
#include "webserv.hpp"

class HttpRequest;
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
		std::string		statusDetails;
		stringmap_t		headerFields;
		octets_t		responseBody;
		octets_t		completeResponse;
		bool			statusLocked;
		std::map<unsigned short,std::string>	codeDict;
		octets_t		message;
		bool			messageTooLongForOneSend;

		void				readRequestedFile(const std::string& targetResource);
		void				readErrorPage(const Location &location);
		void				readReturnDirective(const Location &Location);
		void				readDirectoryListing(const std::string& targetResource);
		void				buildResponseHeaders(const HttpRequest& request);
		void				buildCompleteResponse();

	public:
		HttpResponse();
		HttpResponse(const HttpResponse& refObj);
		HttpResponse& operator = (const HttpResponse& refObj);
		~HttpResponse();

		const octets_t&		getCompleteResponse() const;
		const octets_t		prepareResponse(HttpRequest& request);
		void				setStatusLineAndDetails(const statusLine_t& statusLine, const std::string& details);
		void				lockStatusCode();

		const statusLine_t&		getStatusLine()	const;
		const unsigned short&	getStatusCode() const;
		const bool&				getStatusLocked() const;
		void					setStatusCode(unsigned short code);
		void					updateStatus(unsigned short code, const char* details);
		const octets_t&			getMessage() const;
		void					setMessage(const octets_t& message);
		void					eraseRangeMessage(size_t start, size_t end);
		bool					getMessageTooLongForOneSend() const;
		void					setMessageTooLongForOneSend(bool value);
};

#endif  // HTTPRESPONSE_HPP
