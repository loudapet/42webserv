/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/31 17:11:27 by aulicna           #+#    #+#             */
/*   Updated: 2024/08/05 14:59:24 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
# define LOCATION_HPP

# include "webserv.hpp"
# include "Mime.hpp"
# include "ServerConfig.hpp"

class ServerConfig;

class Location
{
	public:
		Location(void);	// generic Location for when no location was matched
		Location(const ServerConfig &serverConfig); // generic Location for when there is a return directive in it
		Location(std::string locationPath, std::vector<std::string> locationScope); // main Location constructor that parses the input from the config file
		Location(const Location& copy);
		Location	&operator=(const Location &src);
		~Location(void);

		const std::string							&getPath(void) const;
		const std::string							&getRoot(void) const;
		const std::vector<std::string>				&getIndex(void) const;
		int											getRequestBodySizeLimit(void) const;
		int											getAutoindex(void) const;
		const std::set<std::string>					&getAllowMethods(void) const;
		bool										getIsCgi(void) const;
		const std::string							&getReturnURLOrBody(void) const;
		unsigned short								getReturnCode(void) const;
		bool										getIsRedirect(void) const;
		const std::map<unsigned short, std::string>	&getErrorPages(void) const;
		const std::string							&getServerName(void) const;
		const Mime									&getMimeTypes(void) const;
		unsigned short								getPort(void) const;

		void	setPath(const std::string &path);
		void	setRoot(const std::string &root);
		void	setIndex(const std::vector<std::string> &index);
		void	setRequestBodySizeLimit(int requestBodySizeLimit);
		void	setAutoindex(int autoindex);
		void	setAllowMethods(const std::set<std::string> &allowMethods);
		void	setReturnURLOrBody(const std::string &returnURLOrBody);
		void	setReturnCode(unsigned short returnCode);
		void	setIsRedirect(bool value);
		void	addErrorPage(short errorCode, const std::string &errorPageFile);
		void	setServerName(const std::string &serverName);
		void	setMimeTypes(const Mime &mimeTypes);
		void	setPort(unsigned short port);
	
	private:
		
		void	initLocation(void);
		void	validateErrorPagesLine(std::vector<std::string> &errorPageLine);

		std::string								_path;
		std::string								_root;
		std::vector<std::string>				_index;
		int										_requestBodySizeLimit;
		int										_autoindex;
		std::set<std::string>					_allowMethods;
		bool									_isCgi;
		std::string								_returnURLOrBody;
		unsigned short							_returnCode;
		bool									_isRedirect;
		std::map<unsigned short, std::string>	_errorPages;
		std::string								_serverName; // hostname or IP address for CGI
		Mime 									_mimeTypes;
		unsigned short							_port;
};

std::ostream &operator << (std::ostream &o, Location const &instance);

#endif
