/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/31 17:11:10 by aulicna           #+#    #+#             */
/*   Updated: 2024/06/07 11:59:59 by aulicna          ###   ########.fr       */
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
	//std::cout << "Location block: " << locationScope << std::endl;
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
			i += allowMethodsLine.size(); // not -1 bcs there is the directive to skip too
			allowMethodsLine.clear();
		}
		else if (locationScope[i] == "cgi_path" && (i + 1) < locationScope.size())
		{
			this->_cgiPath = extractVectorUntilSemicolon(locationScope, i + 1);
			validateElement(this->_cgiPath.back());
			i += this->_cgiPath.size(); // not -1 bcs there is the directive name to skip too
		}
		else if (locationScope[i] == "cgi_ext" && (i + 1) < locationScope.size())
		{
			this->_cgiExt = extractVectorUntilSemicolon(locationScope, i + 1);
			validateElement(this->_cgiExt.back());
			i += this->_cgiExt.size(); // not -1 bcs there is the directive name to skip too
		}
		else if (locationScope[i] == "return" && (i + 1) < locationScope.size()
			&& validateElement(locationScope[i + 1]))
		{
			this->_return = locationScope[i + 1];
			i++;
		}
		else if (locationScope[i] != "{" && locationScope[i] != "}")
		{
			//std::cout << "unsupported: " << locationScope[i] << std::endl;
			throw (std::runtime_error("Config parser: Invalid directive in a location block."));
		}
	}
	// the location will be completed back in ServerConfig loop over the serverScopeElements as access to the server values is needed
}

Location::Location(const Location& copy)
{
	this->_path = copy.getPath();
	this->_root = copy.getRoot();
	this->_index = copy.getIndex();
	this->_requestBodySizeLimit = copy.getRequestBodySizeLimit();
	this->_autoindex = copy.getAutoindex();
	this->_allowMethods = copy.getAllowMethods();
	this->_cgiPath = copy.getCgiPath();
	this->_cgiExt = copy.getCgiExt();
	this->_cgiMap = copy.getCgiMap();
	this->_return = copy.getReturn();
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
		this->_cgiPath = src.getCgiPath();
		this->_cgiExt = src.getCgiExt();
		this->_cgiMap = src.getCgiMap();
		this->_return = src.getReturn();
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

int	Location::getRequestBodySizeLimit(void) const
{
	return (this->_requestBodySizeLimit);
}

int	Location::getAutoindex(void) const
{
	return (this->_autoindex);	
}

const std::set<std::string>	&Location::getAllowMethods(void) const
{
	return (this->_allowMethods);
}

const std::vector<std::string>		&Location::getCgiPath(void) const
{
	return (this->_cgiPath);
}

const std::vector<std::string>		&Location::getCgiExt(void) const
{
	return (this->_cgiExt);
}

const std::string		&Location::getReturn(void) const
{
	return (this->_return);
}

const std::map<std::string, std::string>	&Location::getCgiMap(void) const
{
	return (this->_cgiMap);
}

void	Location::setRoot(const std::string &root)
{
	this->_root = root;
}

void	Location::setIndex(const std::vector<std::string> &index)
{
	this->_index = index;
}

void	Location::setRequestBodySizeLimit(int requestBodySizeLimit)
{
	this->_requestBodySizeLimit = requestBodySizeLimit;
}

void	Location::setAutoindex(int autoindex)
{
	this->_autoindex = autoindex;
}

void	Location::setCgiMap(std::map<std::string, std::string> &cgiMap)
{
	this->_cgiMap = cgiMap;
}

void	Location::initLocation(void)
{
	this->_path = "";
	this->_root = "";
	this->_index = std::vector<std::string>();
	this->_requestBodySizeLimit = -1;
	this->_autoindex = -1;
	this->_allowMethods = std::set<std::string>();
	this->_cgiPath = std::vector<std::string>();
	this->_cgiExt = std::vector<std::string>();
	this->_cgiMap = std::map<std::string, std::string>();
	this->_return = "";
}

std::ostream &operator << (std::ostream &o, Location const &instance)
{
	std::map<std::string, std::string>	cgiMap;

	cgiMap = instance.getCgiMap();
	o << "*** Location ***" << '\n'
		<< "path: " << instance.getPath() << '\n'
		<< "root: " << instance.getRoot() << '\n'
		<< "index: " << instance.getIndex() << '\n'
		<< "client_max_body_size (requestBodySizeLimit): " << instance.getRequestBodySizeLimit() << '\n'
		<< "autoindex: " << instance.getAutoindex() << '\n'
		<< "allow_methods: " << instance.getAllowMethods() << '\n'
		<< "cgi_path: " << instance.getCgiPath() << '\n'
		<< "cgi_ext: " << instance.getCgiExt() << '\n'
		<< "cgi_map: \n";
	for (std::map<std::string, std::string>::iterator it = cgiMap.begin(); it != cgiMap.end(); it++)
		o << it->first << ": " << it->second << '\n';
	o << "return: " << instance.getReturn();
	return (o);
}