/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/10 12:02:35 by aulicna           #+#    #+#             */
/*   Updated: 2024/06/15 08:49:01 by plouda           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <iostream>
# include <vector>
# include <map>
# include <set>
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

extern bool runWebserv;

typedef std::vector<uint8_t> octets_t;
typedef std::pair<std::string,std::string> stringpair_t;
typedef std::map<std::string,std::string> stringmap_t;
octets_t	convertStringToOctets(std::string& str);
std::string	itoa(size_t num);


# define PORT_SERVER 8000
# define CONNECTION_TIMEOUT 20
//# define CLIENT_MESSAGE_BUFF 4096 // 4 KB
# define CLIENT_MESSAGE_BUFF 8196 // 8 KB
//# define CLIENT_MESSAGE_BUFF 65536 // 64 KB
# define REQUEST_BODY_SIZE_LIMIT 1024 * 1024 // 1 MB

# define WHITESPACES "\t\n\v\f\r "

// Utils.cpp
bool						validateElement(std::string &element);
std::string					validateRoot(const std::string &root, const std::string &locationScopeElement, const std::string &exceptionMessage);
int							validateRequestBodySizeLimit(bool rbslInConfig, const std::string &rbslFromConfig, const std::string &exceptionMessage);
bool						validateAutoindex(bool autoindexInConfig, const std::string &autoindexFromConfig, const std::string &exceptionMessage);
unsigned short				validateListen(unsigned short port, const std::string &portFromConfig);
std::vector<std::string>	validateIndex(const std::vector<std::string> &indexes, const std::vector<std::string> &scopeElements, size_t pos, const std::string &exceptionMessage);

std::vector<std::string>	extractVectorUntilSemicolon(const std::vector<std::string> &mainVector, size_t pos);

void		fileIsValidAndAccessible(const std::string &path, const std::string &exceptionMessage);
std::string	dirIsValidAndAccessible(const std::string &path, const std::string &accessMessage, const std::string &dirOrFileMessage);

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

#endif
