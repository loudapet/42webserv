/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Mime.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/18 13:57:11 by plouda            #+#    #+#             */
/*   Updated: 2024/07/19 13:54:36 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MIME_HPP
#define MIME_HPP
#include "webserv.hpp"

class Mime
{
	private:
		std::map< std::string, std::set<std::string> > _mimeTypesDict;

	public:
		Mime();
		Mime(const Mime& refObj);
		Mime& operator = (const Mime& refObj);
		~Mime();
		
		void		parseMimeTypes(const std::string &mimeTypesFilePath);

		const std::map< std::string, std::set<std::string> >&	getMimeTypesDict() const;
};

#endif  // MIME_HPP
