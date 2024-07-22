/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Mime.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/18 13:57:11 by plouda            #+#    #+#             */
/*   Updated: 2024/07/22 16:46:58 by plouda           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MIME_HPP
#define MIME_HPP
#include "webserv.hpp"

class Mime
{
	private:
		std::map< std::string, std::set<std::string> >	_mimeTypesDict;
		std::map< std::set<std::string>, std::string >	_mimeTypesDictInv;

	public:
		Mime();
		Mime(const Mime& refObj);
		Mime& operator = (const Mime& refObj);
		~Mime();
		
		void		parseMimeTypes(const std::string &mimeTypesFilePath);

		const std::map< std::string, std::set<std::string> >&	getMimeTypesDict() const;
		const std::map< std::set<std::string>, std::string >&	getMimeTypesDictInv() const;
};

#endif  // MIME_HPP
