/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/02 18:05:06 by aulicna           #+#    #+#             */
/*   Updated: 2024/06/02 18:05:28 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

bool	validateElement(std::string &element)
{
	size_t	pos;

	pos = element.rfind(";");
	if (pos != element.size() - 1)
		throw(std::runtime_error("Config parser: Invalid ending of an element in a server block."));
	element.erase(pos);
	return (true);
}
