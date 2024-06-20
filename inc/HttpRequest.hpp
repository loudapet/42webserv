/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/07 11:03:10 by plouda            #+#    #+#             */
/*   Updated: 2024/06/19 13:54:00 by aulicna          ###   ########.fr       */
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
#include <dirent.h>
#include "ServerConfig.hpp"
#include "webserv.hpp"
#include "HttpResponse.hpp"
#include "Location.hpp"
# define CLR1 "\e[38;5;51m"
# define CLR2 "\e[38;5;208m"
# define CLR3 "\e[38;5;213m"
# define CLR4 "\e[38;5;161m"
# define CLR5 "\e[38;5;34m"
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

enum	ConnectionStatus
{
	CLOSE,
	KEEP_ALIVE
};

enum	MessageFraming
{
	NO_CODING,
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

typedef struct RequestLine
{
	std::string			method; // GET, POST, DELETE
	request_target_t	requestTarget; // origin-form / absolute form
	std::string 		httpVersion; // "HTTP/" DIGIT "." DIGIT
} requestLine_t;

typedef struct KeepAlive
{
	int			timeout;
	int			maxRequests;
} keep_alive_t;

class HttpRequest
{
	private:
		requestLine_t			requestLine;
		stringmap_t				headerFields;
		octets_t				requestBody;
		std::string				targetResource; // used to access the resource after URI with location's root
		std::set<std::string>	allowedMethods;
		ConnectionStatus		connectionStatus;
		bool					allowedDirListing;
		bool					isRedirect;
		size_t					contentLength;
		//keep_alive_t			keepAliveParams;
		//bool					interimResponse;
		enum MessageFraming		messageFraming;
		Location				location;
		void					parseRequestLine(std::string requestLine);
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

		HttpResponse			response;
		bool					requestComplete;
		bool					readingBodyInProgress; // false = reading request line and headers, true = reading body
		stringpair_t			parseHeader(octets_t header);
		void					validateHeader(const Location& location);
		size_t					readRequestBody(octets_t bufferedBody);

		void							setConnectionStatus(ConnectionStatus connectionStatus);
		const Location&					getLocation() const;
		const octets_t&					getRequestBody() const;
		const std::string&				getAbsolutePath() const;
		const ConnectionStatus&			getConnectionStatus() const;
		const std::set<std::string>&	getAllowedMethods() const;
		const std::string&				getTargetResource() const;
		void							resetRequestObject(void);
};

std::ostream &operator<<(std::ostream &os, const octets_t &vec);
std::ostream &operator<<(std::ostream &os, octets_t &vec);
std::ostream &operator<<(std::ostream &os, std::vector<octets_t> &vec);
std::ostream &operator<<(std::ostream &os, std::vector<std::string> &vec);
std::ostream &operator<<(std::ostream &os, requestLine_t &requestLine);
std::ostream &operator<<(std::ostream &os, std::map<std::string, std::string> &fieldSection);

typedef void(HttpRequest::*ParseToken)(std::string&);

#endif  // HTTPREQUEST_HPP
