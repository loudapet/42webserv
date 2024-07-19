/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Mime.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/18 13:57:11 by plouda            #+#    #+#             */
/*   Updated: 2024/07/18 15:45:19 by plouda           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MIME_HPP
#define MIME_HPP
#include "webserv.hpp"

class Mime
{
	private:
		std::map< std::string, std::vector<std::string> >	_mimeTypes;
		void		parseMimeTypes();

	public:
		Mime();
		Mime(const Mime& refObj);
		Mime& operator = (const Mime& refObj);
		~Mime();

		const std::map< std::string, std::vector<std::string> >&	getMimeTypes() const;
};

#endif  // MIME_HPP
