/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHeader.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/07 11:03:10 by plouda            #+#    #+#             */
/*   Updated: 2024/05/28 18:00:25 by plouda           ###   ########.fr       */
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
#define DIGITS "0123456789"
#define ALPHA "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPRSTUVWXYZ"
#define TOKEN "!#$%&'*+-.^_`|~"
#define DEFAULT_PORT "8080"

typedef std::vector<uint8_t> octets_t;
octets_t	convertStringToOctets(std::string& str);

enum	MessageType
{
	REQUEST = 1,
	RESPONSE = 2
};

typedef struct RequestTarget
{
	std::pair<std::string,std::string>	authority;
	std::string	host;
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
	private:
		startLine_t							startLine;
		std::map<std::string, std::string>	headerFields;
		void	parseStartLine(std::string startLine);
		void	parseMethod(std::string& token);
		void	parseAuthority(std::string& authority);
		void	parseRequestTarget(std::string& token);
		void	parseHttpVersion(std::string& token);
		void	validateURIElements(void);
		void	resolveDotSegments(std::string& path);
		void	parseFieldSection(std::vector<std::string>& fields);

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
std::ostream &operator<<(std::ostream &os, std::map<std::string, std::string> &fieldSection);


typedef void(HttpHeader::*ParseToken)(std::string&);

#endif  // HTTPHEADER_HPP
