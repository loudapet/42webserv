/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/10 12:02:35 by aulicna           #+#    #+#             */
/*   Updated: 2024/05/18 14:34:06 by aulicna          ###   ########.fr       */
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
# include <netinet/in.h>
# include <arpa/inet.h>
# include <string.h>
# include <cerrno>
# include <fcntl.h>

typedef std::vector<uint8_t> octets_t;

# define PORT_SERVER 8000
# define CONNECTION_TIMEOUT 20
# define CLIENT_MESSAGE_BUFF 10


#endif
