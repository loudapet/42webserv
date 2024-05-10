/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHeader.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/07 11:03:10 by plouda            #+#    #+#             */
/*   Updated: 2024/05/10 12:24:50 by plouda           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPHEADER_HPP
#define HTTPHEADER_HPP
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <map>
#include <stdint.h>
#include <unistd.h>
#include <exception>
#define CR '\r'
#define SP ' '
#define CRLF "\r\n"

typedef std::vector<uint8_t> octets_t;

enum	MessageType
{
	REQUEST = 1,
	RESPONSE = 2
};

typedef struct startLine
{
	octets_t	method; // GET, POST, DELETE
	octets_t	requestTarget; // origin-form / absolute-form
	octets_t 	httpVersion; // "HTTP/" DIGIT "." DIGIT
	octets_t	statusCode; // 1xx/2xx/3xx/4xx/5xx
	octets_t	reasonPhrase; // optional
} startLine_t;

class HttpHeader
{
	protected:
		startLine_t						startLine;
		std::map<octets_t, octets_t>	headerFields;
/* 
		virtual void	parseMethod();
		virtual void	parseRequestTarget();
		virtual void	parseHttpVersion();
		virtual void	parseStatusCode();
		virtual void	parseReasonPhrase();
 */
	public:
		HttpHeader();
		HttpHeader(const HttpHeader& refObj);
		HttpHeader& operator = (const HttpHeader& refObj);
		virtual ~HttpHeader();

		void	parseStartLine(octets_t startLine);
		void	parseHeader(octets_t header);
};

/* class HttpRequest : public HttpHeader
{
	public:
		HttpRequest();
		HttpRequest(const HttpRequest& refObj);
		HttpRequest& operator = (const HttpRequest& refObj);
		~HttpRequest();
}; */

std::ostream &operator<<(std::ostream &os, octets_t &vec);
std::ostream &operator<<(std::ostream &os, std::vector<octets_t> &vec);


#endif  // HTTPHEADER_HPP
