/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/10 12:02:35 by aulicna           #+#    #+#             */
/*   Updated: 2024/06/02 17:55:23 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <iostream>
# include <vector>
# include <map>
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

# define PORT_SERVER 8000
# define CONNECTION_TIMEOUT 20
//# define CLIENT_MESSAGE_BUFF 4096 // 4 KB
# define CLIENT_MESSAGE_BUFF 8196 // 8 KB
//# define CLIENT_MESSAGE_BUFF 65536 // 64 KB
# define REQUEST_BODY_SIZE_LIMIT 1024 * 1024 // 1 MB

# define WHITESPACES "\t\n\v\f\r "

bool	validateElement(std::string &element);

inline std::ostream &operator << (std::ostream &o, std::vector<std::string> &stringVectorToPrint)
{
	for (size_t i = 0; i < stringVectorToPrint.size(); i++)
		o << stringVectorToPrint[i] << std::endl;
	return (o);
}

#endif
