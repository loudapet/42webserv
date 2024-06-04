/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/31 17:11:27 by aulicna           #+#    #+#             */
/*   Updated: 2024/06/04 20:16:17 by aulicna          ###   ########.fr       */
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
		unsigned int	getRequestBodySizeLimit(void) const;
		bool			getAutoindex(void) const;
		const std::set<std::string>	&getAllowMethods(void) const;
		const std::string	&getAlias(void) const;

		void	initLocation(void);
	
	private:
		Location(void);	

		std::string					_path;
		std::string					_root;
		std::vector<std::string>	_index;
		unsigned int				_requestBodySizeLimit;
		bool						_autoindex;
		std::set<std::string>		_allowMethods;
		std::string					_alias;

};

std::ostream &operator << (std::ostream &o, Location const &instance);

#endif
