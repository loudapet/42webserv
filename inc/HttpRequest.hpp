/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/07 11:03:10 by plouda            #+#    #+#             */
/*   Updated: 2024/06/14 13:30:07 by aulicna          ###   ########.fr       */
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
#include <errno.h>
#include <limits.h>
#include <functional>
#include "ServerConfig.hpp"
#include "Location.hpp"
# define CLR1 "\e[38;5;51m"
# define CLR2 "\e[38;5;208m"
# define CLR3 "\e[38;5;213m"
# define CLR4 "\e[38;5;161m"
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

enum	MessageFraming
{
	TRANSFER_ENCODING,
	CONTENT_LENGTH
};

enum	HostLocation
{
	URI,
	HEADER_FIELD
};

typedef struct RequestTarget
{
	stringpair_t 	authority;
	std::string		absolutePath;
	std::string		query;
	std::string		fragment;
} request_target_t;

typedef struct StartLine
{
	std::string			method; // GET, POST, DELETE
	request_target_t	requestTarget; // origin-form / absolute form
	std::string 		httpVersion; // "HTTP/" DIGIT "." DIGIT
} startLine_t;

typedef struct KeepAlive
{
	int			timeout;
	int			maxRequests;
} keep_alive_t;

class HttpRequest
{
	private:
		startLine_t				startLine;
		stringmap_t				headerFields;
		octets_t				requestBody;
		std::string				targetResource; // used to access the resource after URI with location's root
		bool					closeConnection;
		bool					allowedDirListing;
		bool					isRedirect;
		size_t					contentLength;
		//keep_alive_t			keepAliveParams;
		//bool					interimResponse;
		enum MessageFraming		messageFraming;
		void					parseStartLine(std::string startLine);
		void					parseMethod(std::string& token);
		stringpair_t			parseAuthority(std::string& authority, HostLocation parseLocation);
		void					parseRequestTarget(std::string& token);
		void					parseHttpVersion(std::string& token);
		void					validateURIElements(void);
		void					parseFieldSection(std::vector<std::string>& fields);
		stringpair_t			resolveHost();

		void					validateResourceAccess(const Location& location);
		void					validateMessageFraming();
		void					manageExpectations();
		void					validateConnectionOption();

	public:
		HttpRequest();
		HttpRequest(const HttpRequest& refObj);
		HttpRequest& operator = (const HttpRequest& refObj);
		~HttpRequest();

		bool					requestComplete;
		bool					readingBodyInProgress; // false = reading request line and headers, true = reading body
		stringpair_t			parseHeader(octets_t header);
		void					validateHeader(const Location& location);
		size_t					readRequestBody(octets_t bufferedBody);

		const octets_t&			getRequestBody() const;
		const std::string&		getAbsolutePath() const;
};

std::ostream &operator<<(std::ostream &os, const octets_t &vec);
std::ostream &operator<<(std::ostream &os, octets_t &vec);
std::ostream &operator<<(std::ostream &os, std::vector<octets_t> &vec);
std::ostream &operator<<(std::ostream &os, std::vector<std::string> &vec);
std::ostream &operator<<(std::ostream &os, startLine_t &startLine);
std::ostream &operator<<(std::ostream &os, std::map<std::string, std::string> &fieldSection);


typedef void(HttpRequest::*ParseToken)(std::string&);

#endif  // HTTPREQUEST_HPP
