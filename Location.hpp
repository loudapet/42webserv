/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/31 17:11:27 by aulicna           #+#    #+#             */
/*   Updated: 2024/06/03 12:50:10 by aulicna          ###   ########.fr       */
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

		std::string		getPath(void) const;
		std::string		getRoot(void) const;
		std::string		getIndex(void) const;
		unsigned int	getRequestBodySizeLimit(void) const;
		bool			getAutoindex(void) const;

		void	initLocation(void);
	
	private:
		Location(void);

		std::string		_path;
		std::string		_root;
		std::string		_index;
		unsigned int	_requestBodySizeLimit;
		bool			_autoindex;

};

std::ostream &operator << (std::ostream &o, Location const &instance);

#endif
