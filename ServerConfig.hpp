/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/29 12:21:17 by aulicna           #+#    #+#             */
/*   Updated: 2024/06/02 18:15:06 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include "Location.hpp"
# include <iostream>

class Location;

class ServerConfig
{
	public:
		ServerConfig(std::string &serverBlock);
		ServerConfig(const ServerConfig& copy);
		ServerConfig	&operator=(const ServerConfig &src);
		~ServerConfig(void);

		unsigned short					getPort(void) const;
		std::string						getServerName(void) const;
		in_addr_t						getHost(void) const;
		std::string						getRoot(void) const;
		std::string						getIndex(void) const;
		std::map<short, std::string>	getErrorPages(void) const;
		unsigned int					getRequestBodySizeLimit(void) const;
		bool							getAutoindex(void) const;
		std::vector<Location>			getLocations(void) const;

		void	initServerConfig(void);
		void	validateErrorPages(std::vector<std::string> &errorPageLine);

	private:
		ServerConfig(void);

		unsigned short					_port;
		std::string						_serverName;
		in_addr_t						_host;
		std::string						_root;
		std::string						_index;
		std::map<short, std::string> 	_errorPages;
		unsigned int					_requestBodySizeLimit;
		bool							_autoindex;
		std::vector<Location>			_locations;
};

std::ostream &operator << (std::ostream &o, ServerConfig const &instance);

#endif
