/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/07 11:03:10 by plouda            #+#    #+#             */
/*   Updated: 2024/06/04 16:31:44 by plouda           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP
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
#include <sys/stat.h>
#define UNDERLINE "\033[4m"
#define	RESET "\033[0m"
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
typedef std::pair<std::string,std::string> stringpair_t;
typedef std::map<std::string,std::string> stringmap_t;
octets_t	convertStringToOctets(std::string& str);

enum	HostLocation
{
	URI,
	HEADER_FIELD
};

enum	MessageType
{
	REQUEST = 1,
	RESPONSE = 2
};

typedef struct RequestTarget
{
	stringpair_t authority;
	std::string	absolutePath;
	std::string	query;
	std::string	fragment;
} request_target_t;

typedef struct StartLine
{
	std::string			method; // GET, POST, DELETE
	request_target_t	requestTarget; // origin-form / absolute form
	std::string 		httpVersion; // "HTTP/" DIGIT "." DIGIT
} startLine_t;

class HttpRequest
{
	private:
		startLine_t	startLine;
		stringmap_t	headerFields;
		std::string	finalPath; // used to access the resource after URI with location's root
		bool		autoIndexing;
		bool		isRedirect;
		void	parseStartLine(std::string startLine);
		void	parseMethod(std::string& token);
		stringpair_t	parseAuthority(std::string& authority, HostLocation parseLocation);
		void	parseRequestTarget(std::string& token);
		void	parseHttpVersion(std::string& token);
		void	validateURIElements(void);
		void	resolveDotSegments(std::string& path);
		void	parseFieldSection(std::vector<std::string>& fields);
		stringpair_t	resolveHost();

		void	validateResourceAccess();

	public:
		HttpRequest();
		HttpRequest(const HttpRequest& refObj);
		HttpRequest& operator = (const HttpRequest& refObj);
		~HttpRequest();

		stringpair_t	parseHeader(octets_t header);
		void			validateHeader();
};

std::ostream &operator<<(std::ostream &os, octets_t &vec);
std::ostream &operator<<(std::ostream &os, std::vector<octets_t> &vec);
std::ostream &operator<<(std::ostream &os, std::vector<std::string> &vec);
std::ostream &operator<<(std::ostream &os, startLine_t &startLine);
std::ostream &operator<<(std::ostream &os, std::map<std::string, std::string> &fieldSection);


typedef void(HttpRequest::*ParseToken)(std::string&);

#endif  // HTTPREQUEST_HPP
