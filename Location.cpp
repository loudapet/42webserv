/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/31 17:11:10 by aulicna           #+#    #+#             */
/*   Updated: 2024/06/02 18:17:22 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"

Location::Location(void)
{
	return ;
}

Location::Location(std::string locationPath, std::vector<std::string> locationScope)
{
	int								fileCheck; // validate root
	struct stat						fileCheckBuff; // validate root
	char							currentDirectory[4096]; // try to find a valid root
	std::string						tmpRoot; // try to find a valid root

	initLocation();
	this->_path = locationPath;
	for (size_t i = 0; i < locationScope.size(); i++)
	{
		if (locationScope[i] == "root" && (i + 1) < locationScope.size()
			&& validateElement(locationScope[i + 1]))
		{
			if (!this->_root.empty())
				throw (std::runtime_error("Config parser: Duplicate root directive in a location scope."));
			// check the absolute path (raw string from config) and if invalid, check relative path that is interpreted as relative to the current working directory
			fileCheck = stat(locationScope[i + 1].c_str(), &fileCheckBuff);
			if (fileCheck != 0)
				throw(std::runtime_error("Config parser: Cannot access root path of a location."));
			if (!(fileCheckBuff.st_mode & S_IFDIR) || fileCheckBuff.st_mode & S_IFREG) // not a directory, but a file
			{
				getcwd(currentDirectory, sizeof(currentDirectory));
				tmpRoot = currentDirectory + locationScope[i + 1];
				fileCheck = stat(locationScope[i + 1].c_str(), &fileCheckBuff);
				if (!(fileCheckBuff.st_mode & S_IFDIR)) // is not a directory
					throw(std::runtime_error("Config parser: Failed to find a valid root for a location."));
				this->_root = tmpRoot;
			}
			else
				this->_root = locationScope[i + 1];
			i++;
		}
		else if (locationScope[i] == "index" && (i + 1) < locationScope.size()
			&& validateElement(locationScope[i + 1]))
		{
			if (!this->_index.empty())
				throw (std::runtime_error("Config parser: Duplicate index directive in a location scope."));
			this->_index = locationScope[i + 1];
			i++;
		}
	}
	// validate location path - TBA
	// validate index once you have root
	std::cout << "Location created." << std::endl;
	return ;
}

Location::Location(const Location& copy)
{
	this->_path = copy.getPath();
	this->_root = copy.getRoot();
	this->_index = copy.getIndex();
}

Location	&Location::operator = (const Location &src)
{
	if (this != &src)
	{
		this->_path = src.getPath();
		this->_root = src.getRoot();
		this->_index = src.getIndex();
	}
	return (*this);
}

Location::~Location(void)
{
	return ;
}

std::string	Location::getPath(void) const
{
	return (this->_path);

}

std::string	Location::getRoot(void) const
{
	return (this->_root);

}

std::string	Location::getIndex(void) const
{
	return (this->_index);
}

void	Location::initLocation(void)
{
	this->_path = "";
	this->_root = "";
	this->_index = "";
}

std::ostream &operator << (std::ostream &o, Location const &instance)
{
	o << "*** Location ***" << '\n'
		<< "path: " << instance.getPath() << '\n'
		<< "root: " << instance.getRoot() << '\n'
		<< "index: " << instance.getIndex();
	return (o);
}