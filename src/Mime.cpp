/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Mime.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/18 13:57:01 by plouda            #+#    #+#             */
/*   Updated: 2024/07/18 15:01:30 by plouda           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Mime.hpp"
#include "Mime.hpp"

Mime::Mime()
{
	this->_mimeTypes = std::map< std::string, std::vector<std::string> >();
	this->parseMimeTypes();
	return ;
}

Mime::Mime(const Mime& refObj)
{
	*this = refObj;
}

Mime& Mime::operator = (const Mime& refObj)
{
	if (this != &refObj)
		this->_mimeTypes = refObj._mimeTypes;
	return (*this);
}

Mime::~Mime()
{
	return ;
}

const std::map< std::string, std::vector<std::string> > &Mime::getMimeTypes() const
{
	return (this->_mimeTypes);
}

void Mime::parseMimeTypes()
{
	std::string			path = "../config_files/mime_types/mime.types";
	std::string			buff;
	std::stringstream	mimeStr;
	std::ifstream		mimeFile;
	mimeFile.open(path.c_str());
	mimeStr << mimeFile.rdbuf();
	// parse the file
	
	mimeFile.close();
}

