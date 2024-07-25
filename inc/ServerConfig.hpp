/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/29 12:21:17 by aulicna           #+#    #+#             */
/*   Updated: 2024/07/22 17:42:36 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include "Location.hpp"
# include "Mime.hpp"
# include "webserv.hpp"

class ServerConfig
{
	public:
		ServerConfig(void);
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
		const std::vector<std::string>		&getIndex(void) const;
		const std::map<unsigned short, std::string>	&getErrorPages(void) const;
		int									getRequestBodySizeLimit(void) const;
		bool								getAutoindex(void) const;
		const std::set<std::string>			&getAllowMethods(void) const;
		const std::string					&getReturnURLOrBody(void) const;
		unsigned short						getReturnCode(void) const;
		bool								getIsRedirect(void) const;
		const std::vector<Location>			&getLocations(void) const;
		int									getServerSocket(void) const;
		const std::string					&getMimeTypeFile(void) const;

		void startServer(void);

	private:
		void	initServerConfig(void);
		void	validateErrorPagesLine(std::vector<std::string> &errorPageLine);
		void	completeLocations(void);
		void	validateLocations(void);
		
		void	bindSocket(void);


		unsigned short					_port;
		bool							_isDefault;
		std::vector<std::string>		_serverNames;
		std::string						_primaryServerName;
		in_addr_t						_host;
		std::string						_root;
		std::vector<std::string>		_index;
		std::map<unsigned short, std::string> 	_errorPages;
		int								_requestBodySizeLimit;
		bool							_autoindex;
		std::set<std::string>			_allowMethods;
		std::vector<Location>			_locations;
		std::string						_returnURLOrBody;
		unsigned short					_returnCode;
		bool							_isRedirect;
		int								_serverSocket;
		struct sockaddr_in				_serverAddr;
		std::string						_mimeTypesFile;
		Mime 							_mimeTypes;
		bool							_logs;
		std::string						_logsFile;
		std::string						_logsLevel;
		
};

std::ostream &operator << (std::ostream &o, ServerConfig const &instance);

#endif
