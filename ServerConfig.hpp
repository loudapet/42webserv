/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/29 12:21:17 by aulicna           #+#    #+#             */
/*   Updated: 2024/06/03 17:54:35 by aulicna          ###   ########.fr       */
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

		unsigned short						getPort(void) const;
		bool								getIsDefault(void) const;
		const std::string					&getPrimaryServerName(void) const;
		const std::vector<std::string>		&getServerNames(void) const;
		in_addr_t							getHost(void) const;
		const std::string					&getRoot(void) const;
		const std::string					&getIndex(void) const;
		const std::map<short, std::string>	&getErrorPages(void) const;
		unsigned int						getRequestBodySizeLimit(void) const;
		bool								getAutoindex(void) const;
		const std::vector<Location>			&getLocations(void) const;

		void	initServerConfig(void);
		void	validateErrorPagesLine(std::vector<std::string> &errorPageLine);

	private:
		ServerConfig(void);

		unsigned short					_port;
		bool							_isDefault;
		std::vector<std::string>		_serverNames;
		std::string						_primaryServerName;
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
