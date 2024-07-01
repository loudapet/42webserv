/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/31 17:11:27 by aulicna           #+#    #+#             */
/*   Updated: 2024/07/01 17:01:13 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
# define LOCATION_HPP

# include "webserv.hpp"

class Location
{
	public:
		Location(void);	
		Location(unsigned short serverReturnCode, std::string serverReturnURL);
		Location(std::string locationPath, std::vector<std::string> locationScope);
		Location(const Location& copy);
		Location	&operator=(const Location &src);
		~Location(void);

		const std::string							&getPath(void) const;
		const std::string							&getRoot(void) const;
		const std::vector<std::string>				&getIndex(void) const;
		int											getRequestBodySizeLimit(void) const;
		int											getAutoindex(void) const;
		const std::set<std::string>					&getAllowMethods(void) const;
		const std::string							&getCgiPath(void) const;
		const std::string							&getRelativeCgiPath(void) const;
		const std::string							&getReturnURLOrBody(void) const;
		unsigned short								getReturnCode(void) const;
		bool										getIsRedirect(void) const;
		const std::map<unsigned short, std::string>	&getErrorPages(void) const;

		void	setPath(const std::string &path);
		void	setRoot(const std::string &root);
		void	setIndex(const std::vector<std::string> &index);
		void	setRequestBodySizeLimit(int requestBodySizeLimit);
		void	setAutoindex(int autoindex);
		void	setAllowMethods(const std::set<std::string> &allowMethods);
		void	setRelativeCgiPath(const std::string &relativeCgiPath);
		void	setReturnURLOrBody(const std::string &returnURLOrBody);
		void	setReturnCode(unsigned short returnCode);
		void	setIsRedirect(bool value);
		void	addErrorPage(short errorCode, const std::string &errorPageFile);
	
	private:
		
		void	initLocation(void);
		void	validateErrorPagesLine(std::vector<std::string> &errorPageLine);

		std::string								_path;
		std::string								_root;
		std::vector<std::string>				_index;
		int										_requestBodySizeLimit;
		int										_autoindex;
		std::set<std::string>					_allowMethods;
		std::string								_cgiPath;
		std::string								_relativeCgiPath;
		std::string								_returnURLOrBody;
		unsigned short							_returnCode;
		bool									_isRedirect;
		std::map<unsigned short, std::string>	_errorPages;
};

std::ostream &operator << (std::ostream &o, Location const &instance);

#endif
