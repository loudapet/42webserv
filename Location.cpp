/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/31 17:11:10 by aulicna           #+#    #+#             */
/*   Updated: 2024/06/03 16:50:48 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"

Location::Location(void)
{
	return ;
}

Location::Location(std::string locationPath, std::vector<std::string> locationScope)
{
	bool						rbslInConfig;
	bool						autoindexInConfig;
	std::vector<std::string>	allowMethodsLine;

	initLocation();
	rbslInConfig = false;
	autoindexInConfig = false;
	this->_path = locationPath;
	for (size_t i = 0; i < locationScope.size(); i++)
	{
		if (locationScope[i] == "root" && (i + 1) < locationScope.size()
			&& validateElement(locationScope[i + 1]))
		{
			this->_root = validateRoot(this->_root, locationScope[i + 1], "location");
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
		else if (locationScope[i] == "client_max_body_size" && (i + 1) < locationScope.size()
			&& validateElement(locationScope[i + 1]))
		{
			this->_requestBodySizeLimit = validateRequestBodySizeLimit(rbslInConfig, locationScope[i + 1], "location"); 
			rbslInConfig = true;
			i++;
		}
		else if (locationScope[i] == "autoindex" && (i + 1) < locationScope.size()
			&& validateElement(locationScope[i + 1]))
		{
			this->_autoindex = validateAutoindex(autoindexInConfig, locationScope[i + 1], "location");
			autoindexInConfig = true;
			i++;
		}
		else if (locationScope[i] == "allow_methods")
		{
			size_t j = 0;
			while (locationScope[i + j].find(';') == std::string::npos && i + j < locationScope.size())
			{
				allowMethodsLine.push_back(locationScope[i + j]);
				j++;
			}
			allowMethodsLine.push_back(locationScope[i + j]);
	//		validateAllowMethodsLine(allowMethodsLine);
			i += allowMethodsLine.size() - 1;
			allowMethodsLine.clear();
		}

	}
	// validate location path - TBA
	// validate index once you have root
	// what if difference between server and location scope directives - e.g. autoindex off in server but off in location
	// require methods?
}

Location::Location(const Location& copy)
{
	this->_path = copy.getPath();
	this->_root = copy.getRoot();
	this->_index = copy.getIndex();
	this->_requestBodySizeLimit = copy.getRequestBodySizeLimit();
	this->_autoindex = copy.getAutoindex();
}

Location	&Location::operator = (const Location &src)
{
	if (this != &src)
	{
		this->_path = src.getPath();
		this->_root = src.getRoot();
		this->_index = src.getIndex();
		this->_requestBodySizeLimit = src.getRequestBodySizeLimit();
		this->_autoindex = src.getAutoindex();
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

unsigned int	Location::getRequestBodySizeLimit(void) const
{
	return (this->_requestBodySizeLimit);
}

bool	Location::getAutoindex(void) const
{
	return (this->_autoindex);	
}

void	Location::initLocation(void)
{
	this->_path = "";
	this->_root = "";
	this->_index = "";
	this->_requestBodySizeLimit = REQUEST_BODY_SIZE_LIMIT;
	this->_autoindex = false;
}

std::ostream &operator << (std::ostream &o, Location const &instance)
{
	o << "*** Location ***" << '\n'
		<< "path: " << instance.getPath() << '\n'
		<< "root: " << instance.getRoot() << '\n'
		<< "index: " << instance.getIndex() << '\n'
		<< "client_max_body_size (requestBodySizeLimit): " << instance.getRequestBodySizeLimit() << '\n'
		<< "autoindex: " << instance.getAutoindex();
	return (o);
}