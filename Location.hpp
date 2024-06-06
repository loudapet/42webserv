/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/31 17:11:27 by aulicna           #+#    #+#             */
/*   Updated: 2024/06/06 21:37:56 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
# define LOCATION_HPP

# include "webserv.hpp"

class Location
{
	public:
		Location(std::string locationPath, std::vector<std::string> locationScope);
		Location(const Location& copy);
		Location	&operator=(const Location &src);
		~Location(void);

		const std::string		&getPath(void) const;
		const std::string		&getRoot(void) const;
		const std::vector<std::string>		&getIndex(void) const;
		int	getRequestBodySizeLimit(void) const;
		int				getAutoindex(void) const;
		const std::set<std::string>	&getAllowMethods(void) const;
		const std::vector<std::string>		&getCgiPath(void) const;
		const std::vector<std::string>		&getCgiExt(void) const;
		const std::string		&getReturn(void) const;
		const std::map<std::string, std::string>	&getCgiMap(void) const;

		void	setRoot(const std::string &root);
		void	setIndex(const std::vector<std::string> &index);
		void	setRequestBodySizeLimit(int requestBodySizeLimit);
		void	setAutoindex(int autoindex);
		void	setCgiMap(std::map<std::string, std::string> &cgiMap);

		void	initLocation(void);
	
	private:
		Location(void);	

		std::string					_path;
		std::string					_root;
		std::vector<std::string>	_index;
		int							_requestBodySizeLimit;
		int							_autoindex;
		std::set<std::string>		_allowMethods;
		std::vector<std::string>	_cgiPath;
		std::vector<std::string>	_cgiExt;
		std::map<std::string, std::string>	_cgiMap;
		std::string					_return;

};

std::ostream &operator << (std::ostream &o, Location const &instance);

#endif
