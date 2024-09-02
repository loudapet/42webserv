/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/07 11:03:10 by plouda            #+#    #+#             */
/*   Updated: 2024/09/02 10:21:25 by aulicna          ###   ########.fr       */
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
	std::string			method; // GET, HEAD, POST, DELETE
	request_target_t	requestTarget; // origin-form / absolute form
	std::string 		httpVersion; // DIGIT "." DIGIT
} requestLine_t;

typedef struct KeepAlive
{
	int			timeout;
	int			maxRequests;
} keep_alive_t;

class HttpRequest
{
	private:
		//int						requestID;
		requestLine_t			requestLine;
		stringmap_t				headerFields;
		octets_t				requestBody;
		int						requestBodySizeLimit;
		std::string				targetResource; // used to access the resource after URI with location's root
		std::string				cgiPathInfo; // path after the script
		bool					targetIsDirectory;
		std::set<std::string>	allowedMethods;
		ConnectionStatus		connectionStatus;
		MessageFraming			messageFraming;
		bool					allowedDirListing;
		bool					isRedirect;
		bool					hasExpect;
		bool					silentErrorRaised;
		bool					isCgiExec;
		size_t					contentLength;
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
		void					validateContentType(const Location& location);

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

		void							setRequestBody(octets_t requestBody);
		void							setConnectionStatus(ConnectionStatus connectionStatus);
		const Location&					getLocation() const;
		const requestLine_t&			getRequestLine() const;
		const stringmap_t&				getHeaderFields() const;
		octets_t&						getRequestBody();
		const std::string&				getAbsolutePath() const;
		const ConnectionStatus&			getConnectionStatus() const;
		const std::set<std::string>&	getAllowedMethods() const;
		const std::string&				getTargetResource() const;
		const std::string&				getCgiPathInfo() const;
		const bool&						getTargetIsDirectory() const;
		const int&						getRequestID() const;
		void							resetRequestObject(void);
		bool							getHasExpect() const;
		bool							getIsCgiExec() const;
		void							disableHasExpect();
};

std::ostream &operator<<(std::ostream &os, const octets_t &vec);
std::ostream &operator<<(std::ostream &os, octets_t &vec);
std::ostream &operator<<(std::ostream &os, std::vector<octets_t> &vec);
std::ostream &operator<<(std::ostream &os, std::vector<std::string> &vec);
std::ostream &operator<<(std::ostream &os, requestLine_t &requestLine);
std::ostream &operator<<(std::ostream &os, const requestLine_t &requestLine);
std::ostream &operator<<(std::ostream &os, std::map<std::string, std::string> &fieldSection);

typedef void(HttpRequest::*ParseToken)(std::string&);

#endif  // HTTPREQUEST_HPP
