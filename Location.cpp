/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/31 17:11:10 by aulicna           #+#    #+#             */
/*   Updated: 2024/06/04 20:58:33 by aulicna          ###   ########.fr       */
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
	bool						allowMethodsInConfig;
	std::vector<std::string>	allowMethodsLine;
	std::string 				validMethodsArray[] = {"GET", "POST", "DELETE"};
	std::set<std::string> 		validMethods(validMethodsArray, validMethodsArray + sizeof(validMethodsArray) / sizeof(validMethodsArray[0]));

	initLocation();
	rbslInConfig = false;
	autoindexInConfig = false;
	allowMethodsInConfig = false;
	this->_path = locationPath;
	for (size_t i = 0; i < locationScope.size(); i++)
	{
		if (locationScope[i] == "root" && (i + 1) < locationScope.size()
			&& validateElement(locationScope[i + 1]))
		{
			this->_root = validateRoot(this->_root, locationScope[i + 1], "location");
			i++;
		}
		else if (locationScope[i] == "index" && (i + 1) < locationScope.size())
		{
			this->_index = validateIndex(this->_index, locationScope, i + 1, "location");
			i += this->_index.size(); // not -1 bcs there is the directive name to skip too
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
			if (allowMethodsInConfig)
				throw(std::runtime_error("Config parser: Duplicate allow_methods directive in a location block."));
			allowMethodsLine = extractVectorUntilSemicolon(locationScope, i + 1);
			validateElement(allowMethodsLine.back());
			for (size_t i = 0; i < allowMethodsLine.size(); i++)
			{
				if (validMethods.find(allowMethodsLine[i]) == validMethods.end())
					throw(std::runtime_error("Config parser: Invalid method '" + allowMethodsLine[i] + "'."));
				if (!this->_allowMethods.insert(allowMethodsLine[i]).second)
					throw(std::runtime_error("Config parser: Duplicate method '" + allowMethodsLine[i] + "'."));
				this->_allowMethods.insert(allowMethodsLine[i]);
			}
			i += allowMethodsLine.size() - 1;
			allowMethodsLine.clear();
		}
		else if (locationScope[i] == "alias" && (i + 1) < locationScope.size()
			&& validateElement(locationScope[i + 1]))
		{
			// QUESTION: not sure how (and therefore where) to validate? With root as prefix? Need to check if the location has a root and if not fallback on the server one?
			//this->_alias = dirIsValidAndAccessible(locationScope[i + 1], "alias", "location");
			this->_alias = locationScope[i + 1];
			if (this->_alias.length() >= this->_path.length() &&
				   this->_alias.compare(this->_alias.size() - this->_path.length(), this->_path.length(), this->_path) == 0)
				throw(std::runtime_error("Config parser: The location path matches the last part of the alias directive value. Please use the root directive instead."));
			i++;
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
	this->_allowMethods = copy.getAllowMethods();
	this->_alias = copy.getAlias();
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
		this->_allowMethods = src.getAllowMethods();
		this->_alias = src.getAlias();
	}
	return (*this);
}

Location::~Location(void)
{
	return ;
}

const std::string	&Location::getPath(void) const
{
	return (this->_path);

}

const std::string	&Location::getRoot(void) const
{
	return (this->_root);

}

const std::vector<std::string>	&Location::getIndex(void) const
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

const std::set<std::string>	&Location::getAllowMethods(void) const
{
	return (this->_allowMethods);
}

const std::string	&Location::getAlias(void) const
{
	return (this->_alias);
}

void	Location::initLocation(void)
{
	this->_path = "";
	this->_root = "";
	this->_index = std::vector<std::string>();
	this->_requestBodySizeLimit = REQUEST_BODY_SIZE_LIMIT;
	this->_autoindex = false;
	this->_allowMethods = std::set<std::string>();
}

std::ostream &operator << (std::ostream &o, Location const &instance)
{
	o << "*** Location ***" << '\n'
		<< "path: " << instance.getPath() << '\n'
		<< "root: " << instance.getRoot() << '\n'
		<< "index: " << instance.getIndex() << '\n'
		<< "client_max_body_size (requestBodySizeLimit): " << instance.getRequestBodySizeLimit() << '\n'
		<< "autoindex: " << instance.getAutoindex() << '\n'
		<< "allow_methods: " << instance.getAllowMethods() << '\n'
		<< "alias: " << instance.getAlias();
	return (o);
}