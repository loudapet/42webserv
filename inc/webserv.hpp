/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okraus <okraus@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/10 12:02:35 by aulicna           #+#    #+#             */
/*   Updated: 2024/08/30 09:20:18 by okraus           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <iostream>
# include <vector>
# include <map>
# include <set>
# include <stack>
# include <unistd.h>
# include <stdexcept>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/stat.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <string.h>
# include <cerrno>
# include <fcntl.h>
# include <algorithm>
# include <signal.h>
# include <fstream>
# include <sstream>
# include <arpa/inet.h>
# include <limits.h>

# define CLR1 "\e[38;5;51m"
# define CLR2 "\e[38;5;208m"
# define CLR3 "\e[38;5;213m"
# define CLR4 "\e[38;5;161m"
# define CLR5 "\e[38;5;34m"
# define CLR6 "\e[38;5;226m"
# define CLRE "\e[38:5:226;48:5:196m"
# define UNDERLINE "\033[4m"
# define	RESET "\033[0m"
enum ServerSection
{
	CONFIG,
	REQUEST,
	RESPONSE,
	CGI,
	SERVER
};
# include "../inc/Logger.hpp"

extern bool g_runWebserv;

// enum	DotSegmentsResolution
// {
// 	CONFIG,
// 	REQUEST
// };



typedef std::vector<uint8_t> octets_t;
typedef std::pair<std::string,std::string> stringpair_t;
typedef std::map<std::string,std::string> stringmap_t;
octets_t	convertStringToOctets(std::string str);
std::string convertOctetsToString(const octets_t& octets);

# define PORT_SERVER 8000
# define CONNECTION_TIMEOUT 120
# define VALID_HEADER_TIMEOUT 230
//# define CLIENT_MESSAGE_BUFF 4096 // 4 KB
//# define CLIENT_MESSAGE_BUFF 8196 // 8 KB
# define CLIENT_MESSAGE_BUFF 8196000 // 8 KB
//# define CLIENT_MESSAGE_BUFF 65536 // 64 KB
# define REQUEST_BODY_SIZE_LIMIT 1024 * 1024 * 1024 // 1 MB

# define WHITESPACES "\t\n\v\f\r "

// Utils.cpp
bool						validateElement(std::string &element);
std::string					validateRoot(const std::string &root, const std::string &locationScopeElement, const std::string &exceptionMessage);
int							validateRequestBodySizeLimit(bool rbslInConfig, const std::string &rbslFromConfig, const std::string &exceptionMessage);
bool						validateOnOffDirective(bool isDirectiveInConfig, const std::string &directiveFromConfig, const std::string &directiveValueFromConfig, const std::string &exceptionMessage);
unsigned short				validateListen(unsigned short port, const std::string &portFromConfig);
std::vector<std::string>	validateIndex(const std::vector<std::string> &indexes, const std::vector<std::string> &scopeElements, size_t pos, const std::string &exceptionMessage);
unsigned short				validateReturnCode(std::string &scopeElement);
std::vector<std::string>	extractVectorUntilSemicolon(const std::vector<std::string> &mainVector, size_t pos);

void						fileIsValidAndAccessible(const std::string &path, const std::string &fileName);
std::string					dirIsValidAndAccessible(const std::string &path, const std::string &accessMessage, const std::string &dirOrFileMessage);
std::string					resolveDotSegments(std::string path, ServerSection flag);
void						logSockets(int socket, std::string action);
std::string					trim(const std::string& str);
std::vector<std::string>	splitQuotedString(const std::string& str, char sep);
std::vector<std::string>	splitBlock(std::string &block);

/* inline std::ostream &operator << (std::ostream &o, std::vector<std::string> &stringVectorToPrint)
{
	for (size_t i = 0; i < stringVectorToPrint.size(); i++)
		o << stringVectorToPrint[i] << std::endl;
	return (o);
} */

inline std::ostream &operator << (std::ostream &o, const std::vector<std::string> &stringVectorToPrint)
{
	for (size_t i = 0; i < stringVectorToPrint.size(); i++)
		o << stringVectorToPrint[i] << " ";
	return (o);
}

inline std::ostream &operator << (std::ostream &o, const std::set<std::string> &stringSetToPrint)
{
	for (std::set<std::string>::const_iterator it = stringSetToPrint.begin(); it != stringSetToPrint.end(); ++it)
		o << *it << " ";
	return (o);
}

template <typename T> std::string	itoa(T num)
{
	std::stringstream ss;
    ss << num;
	return (std::string(ss.str()));
}

#endif
