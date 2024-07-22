/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/31 17:11:10 by aulicna           #+#    #+#             */
/*   Updated: 2024/07/22 09:48:57 by plouda           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Location.hpp"

Location::Location(void)
{
	this->_path = "";
	this->_root = "./";
	this->_index = std::vector<std::string>(1, "index.html");
	this->_requestBodySizeLimit = REQUEST_BODY_SIZE_LIMIT;
	this->_autoindex = 0;
	this->_allowMethods.insert("GET");
	this->_isCgi = false;
	this->_returnURLOrBody = "/";
	this->_returnCode = 0;
	this->_isRedirect = false;
	this->_errorPages = std::map<unsigned short, std::string>();
	this->_serverName = "";
	this->_mimeTypes = Mime();
}

Location::Location(unsigned short serverReturnCode, std::string serverReturnURLOrBody)
{
	this->_path = "";
	this->_root = "./";
	this->_index = std::vector<std::string>(1, "index.html");
	this->_requestBodySizeLimit = REQUEST_BODY_SIZE_LIMIT;
	this->_autoindex = 0;
	this->_allowMethods.insert("GET");
	this->_isCgi = false;
	this->_returnURLOrBody = serverReturnURLOrBody;
	this->_returnCode = serverReturnCode;
	this->_isRedirect = true;
	this->_errorPages = std::map<unsigned short, std::string>();
	this->_serverName = "";
	this->_mimeTypes = Mime();
}

Location::Location(std::string locationPath, std::vector<std::string> locationBlockElements)
{
	bool						rbslInConfig;
	bool						autoindexInConfig;
	bool						allowMethodsInConfig;
	bool						isCgiInConfig;
	std::vector<std::string>	allowMethodsLine;
	std::string 				validMethodsArray[] = {"GET", "POST", "DELETE"};
	std::set<std::string> 		validMethods(validMethodsArray, validMethodsArray + sizeof(validMethodsArray) / sizeof(validMethodsArray[0]));
	std::vector<std::string>	errorPageLine; // to validate error page lines
	std::istringstream			iss; // convert error code to short

	initLocation();
	rbslInConfig = false;
	autoindexInConfig = false;
	allowMethodsInConfig = false;
	isCgiInConfig = false;
	this->_path = locationPath;
	//std::cout << "Location block: " << locationScope << std::endl;
	for (size_t i = 0; i < locationBlockElements.size(); i++)
	{
		if (locationBlockElements[i] == "root" && (i + 1) < locationBlockElements.size()
			&& validateElement(locationBlockElements[i + 1]))
		{
			this->_root = validateRoot(this->_root, locationBlockElements[i + 1], "location");
			i++;
		}
		else if (locationBlockElements[i] == "index" && (i + 1) < locationBlockElements.size())
		{
			this->_index = validateIndex(this->_index, locationBlockElements, i + 1, "location");
			i += this->_index.size(); // not -1 bcs there is the directive name to skip too
		}
		else if (locationBlockElements[i] == "client_max_body_size" && (i + 1) < locationBlockElements.size()
			&& validateElement(locationBlockElements[i + 1]))
		{
			this->_requestBodySizeLimit = validateRequestBodySizeLimit(rbslInConfig, locationBlockElements[i + 1], "location"); 
			rbslInConfig = true;
			i++;
		}
		else if (locationBlockElements[i] == "autoindex" && (i + 1) < locationBlockElements.size()
			&& validateElement(locationBlockElements[i + 1]))
		{
			this->_autoindex = validateOnOffDirective(autoindexInConfig, "autoindex", locationBlockElements[i + 1], "location");
			autoindexInConfig = true;
			i++;
		}
		else if (locationBlockElements[i] == "allow_methods")
		{
			if (allowMethodsInConfig)
				throw(std::runtime_error("Config parser: Duplicate allow_methods directive in a location block."));
			allowMethodsLine = extractVectorUntilSemicolon(locationBlockElements, i + 1);
			validateElement(allowMethodsLine.back());
			for (size_t i = 0; i < allowMethodsLine.size(); i++)
			{
				if (validMethods.find(allowMethodsLine[i]) == validMethods.end())
					throw(std::runtime_error("Config parser: Invalid method '" + allowMethodsLine[i] + "'."));
				if (!this->_allowMethods.insert(allowMethodsLine[i]).second)
					throw(std::runtime_error("Config parser: Duplicate method '" + allowMethodsLine[i] + "'."));
			}
			i += allowMethodsLine.size(); // not -1 bcs there is the directive to skip too
			allowMethodsLine.clear();
		}
		else if (locationBlockElements[i] == "cgi" && (i + 1) < locationBlockElements.size()
			&& validateElement(locationBlockElements[i + 1]))
		{
			this->_isCgi = validateOnOffDirective(isCgiInConfig, "cgi", locationBlockElements[i + 1], "location");
			isCgiInConfig = true;
			i++;
		}
		else if (locationBlockElements[i] == "return" && (i + 2) < locationBlockElements.size()
			&& locationBlockElements[i + 1].find_first_of(";") == std::string::npos && validateElement(locationBlockElements[i + 2]))
		{
			// source: https://nginx.org/en/docs/http/ngx_http_rewrite_module.html#return
			this->_returnCode = validateReturnCode(locationBlockElements[i + 1]);
			this->_returnURLOrBody = locationBlockElements[i + 2];		
			this->_isRedirect = true;
			i += 2;						
		}
		else if (locationBlockElements[i] == "return" && (i + 1) < locationBlockElements.size()
			&& validateElement(locationBlockElements[i + 1]))
		{
			this->_returnCode = 302;
			this->_returnURLOrBody = locationBlockElements[i + 1];
			if (this->_returnURLOrBody.substr(0, 7) != "http://" && this->_returnURLOrBody.substr(0, 8) != "https://")
				throw(std::runtime_error("Config parser: Invalid URL in return directive. A URL for temporary redirect with the code 302 should start with the 'http://' or 'https://'."));
			this->_isRedirect = true;
			i++;
		}
		else if (locationBlockElements[i] == "error_page")
		{
			errorPageLine = extractVectorUntilSemicolon(locationBlockElements, i);
			validateErrorPagesLine(errorPageLine);
			i += errorPageLine.size() - 1;
			errorPageLine.clear();
		}
		else if (locationBlockElements[i] != "{" && locationBlockElements[i] != "}")
		{
			//std::cout << "unsupported: " << locationScope[i] << std::endl;
			throw (std::runtime_error("Config parser: Invalid directive in a location block."));
		}
	}
	// the location will be completed back in ServerConfig loop over the serverScopeElements as access to the server values is needed
}

Location::Location(const Location& copy)
{
	this->_path = copy._path;
	this->_root = copy._root;
	this->_index = copy._index;
	this->_requestBodySizeLimit = copy._requestBodySizeLimit;
	this->_autoindex = copy._autoindex;
	this->_allowMethods = copy._allowMethods;
	this->_isCgi = copy._isCgi;
	this->_returnURLOrBody = copy._returnURLOrBody;
	this->_returnCode = copy._returnCode;
	this->_isRedirect = copy._isRedirect;
	this->_errorPages = copy._errorPages;
	this->_serverName = copy._serverName;
	this->_mimeTypes = copy._mimeTypes;
}

Location &Location::operator = (const Location &src)
{
	if (this != &src)
	{
		this->_path = src._path;
		this->_root = src._root;
		this->_index = src._index;
		this->_requestBodySizeLimit = src._requestBodySizeLimit;
		this->_autoindex = src._autoindex;
		this->_allowMethods = src._allowMethods;
		this->_isCgi = src._isCgi;
		this->_returnURLOrBody = src._returnURLOrBody;
		this->_returnCode = src._returnCode;
		this->_isRedirect = src._isRedirect;
		this->_errorPages = src._errorPages;
		this->_serverName = src._serverName;
		this->_mimeTypes = src._mimeTypes;
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

bool	Location::getIsCgi(void) const
{
	return (this->_isCgi);
}

const std::string		&Location::getReturnURLOrBody(void) const
{
	return (this->_returnURLOrBody);
}

unsigned short	Location::getReturnCode(void) const
{
	return (this->_returnCode);
}

bool	Location::getIsRedirect(void) const
{
	return (this->_isRedirect);
}

const std::map<unsigned short, std::string>	&Location::getErrorPages(void) const
{
	return (this->_errorPages);
}

const std::string   &Location::getServerName(void) const
{
    return (this->_serverName);
}

const Mime	&Location::getMimeTypes(void) const
{
	return (this->_mimeTypes);
}

void	Location::setPath(const std::string &path)
{
	this->_path = path;
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

void	Location::setAllowMethods(const std::set<std::string> &allowMethods)
{
	this->_allowMethods = allowMethods;
}

void	Location::setReturnURLOrBody(const std::string &returnURLOrBody)
{
	this->_returnURLOrBody = returnURLOrBody; 
}

void	Location::setReturnCode(unsigned short returnCode)
{
	this->_returnCode = returnCode; 
}

void	Location::setIsRedirect(bool value)
{
	this->_isRedirect = value; 
}

void	Location::addErrorPage(short errorCode, const std::string &errorPageFile)
{
	this->_errorPages[errorCode] = errorPageFile;
}

void	Location::setServerName(const std::string &serverName)
{
	this->_serverName = serverName;
}

void	Location::setMimeTypes(const Mime &mimeTypes)
{
	this->_mimeTypes = mimeTypes;
}

void	Location::initLocation(void)
{
	this->_path = "";
	this->_root = "";
	this->_index = std::vector<std::string>();
	this->_requestBodySizeLimit = -1;
	this->_autoindex = -1;
	this->_allowMethods = std::set<std::string>();
	this->_isCgi = false;
	this->_returnURLOrBody = "/";
	this->_returnCode = 0;
	this->_isRedirect = false;
	this->_errorPages = std::map<unsigned short, std::string>();
	this->_serverName = "";
	this->_mimeTypes = Mime();
}
		
void	Location::validateErrorPagesLine(std::vector<std::string> &errorPageLine)
{
	short							tmpErrorCode;
	std::istringstream				iss; // convert error code to short
	std::string						errorPageFileName;

	if (errorPageLine.size() < 3)
		throw(std::runtime_error("Config parser (location): Invalid formatting of error_page directive."));
	errorPageFileName = errorPageLine.back();
	if (validateElement(errorPageFileName))
	{
		for (size_t i = 1; i < errorPageLine.size() - 1; i++) // -1 to ignore the page itself
		{
			for (size_t j = 0; j < errorPageLine[i].size(); j++)
			{
				if (!std::isdigit(errorPageLine[i][j]))
					throw (std::runtime_error("Config parser (location): Invalid error page number."));
			}
			iss.str(errorPageLine[i]);
			if (!(iss >> tmpErrorCode) || !iss.eof())
				throw(std::runtime_error("Config parser (location): Error page number is out of range for short."));
			iss.str("");
			iss.clear();
			if (tmpErrorCode < 400 || (tmpErrorCode > 426 && tmpErrorCode < 500) || tmpErrorCode > 505)
				throw(std::runtime_error("Config parser (location): Error page number is out of range of valid error pages."));
			this->_errorPages[tmpErrorCode] = errorPageFileName;
		}
	}
}

std::ostream &operator << (std::ostream &o, Location const &instance)
{
	std::map<unsigned short, std::string>		errorPages;

	errorPages = instance.getErrorPages();
	o << "*** Location ***" << '\n'
		<< "path: " << instance.getPath() << '\n'
		<< "root: " << instance.getRoot() << '\n'
		<< "index: " << instance.getIndex() << '\n'
		<< "client_max_body_size (requestBodySizeLimit): " << instance.getRequestBodySizeLimit() << '\n'
		<< "autoindex: " << instance.getAutoindex() << '\n'
		<< "allow_methods: " << instance.getAllowMethods() << '\n'
		<< "isCgi: " << instance.getIsCgi() << '\n'
		<< "return: " << instance.getReturnCode() << " " << instance.getReturnURLOrBody() << '\n'
		<< "error pages: ";
	if (errorPages.size() > 0)
		o << "\n";
	for (std::map<unsigned short, std::string>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it)
	{
		o << it->first << ": " << it->second;
		std::map<unsigned short, std::string>::const_iterator next_it = it;
		next_it++;
		if (next_it != errorPages.end())
			o << '\n';
	}
	return (o);
}