/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHeader.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/07 11:03:10 by plouda            #+#    #+#             */
/*   Updated: 2024/05/17 12:33:16 by plouda           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPHEADER_HPP
#define HTTPHEADER_HPP
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <map>
#include <stack>
#include <stdint.h>
#include <unistd.h>
#include <exception>
#include <stdlib.h>
#define CR '\r'
#define SP ' '
#define CRLF "\r\n"
#define UNRESERVED "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPRSTUVWXYZ1234567890-._~"
#define HEXDIGITS "1234567890ABCDEFabcdef"
#define PCHAR_EXTRA ":@/"
#define SUBDELIMS "!$&'()*+,;="

typedef std::vector<uint8_t> octets_t;
octets_t	convertStringToOctets(std::string& str);

enum	MessageType
{
	REQUEST = 1,
	RESPONSE = 2
};

typedef struct RequestTarget
{
	std::string	absolutePath;
	std::string	query;
	std::string	fragment;
} request_target_t;

typedef struct startLine
{
	std::string			method; // GET, POST, DELETE
	request_target_t	requestTarget; // origin-form
	std::string 		httpVersion; // "HTTP/" DIGIT "." DIGIT
	std::string			statusCode; // 1xx/2xx/3xx/4xx/5xx
	std::string			reasonPhrase; // optional
} startLine_t;

class HttpHeader
{
	protected:
		startLine_t						startLine;
		std::map<octets_t, octets_t>	headerFields;
		void	parseStartLine(std::string startLine);
		void	parseMethod(std::string& token);
		void	parseRequestTarget(std::string& token);
		void	parseHttpVersion(std::string& token);
		void	validateURIElements(void);
		void	resolveDotSegments(std::string& path);

	public:
		HttpHeader();
		HttpHeader(const HttpHeader& refObj);
		HttpHeader& operator = (const HttpHeader& refObj);
		~HttpHeader();

		void	parseHeader(octets_t header);
};

std::ostream &operator<<(std::ostream &os, octets_t &vec);
std::ostream &operator<<(std::ostream &os, std::vector<octets_t> &vec);
std::ostream &operator<<(std::ostream &os, std::vector<std::string> &vec);
std::ostream &operator<<(std::ostream &os, startLine_t &startLine);


typedef void(HttpHeader::*ParseToken)(std::string&);

#endif  // HTTPHEADER_HPP
