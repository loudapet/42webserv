/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/22 18:27:49 by aulicna           #+#    #+#             */
/*   Updated: 2024/05/23 16:08:26 aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"

ServerConfig::ServerConfig(void)
{
	return ;
}

ServerConfig::ServerConfig(std::string configFile)
{
	int				fileCheck;
	struct stat		fileCheckBuff;
	std::ifstream	file;
	char			c;
	std::stringstream	tmpFileContent;

	if (configFile.size() < 5 || configFile.substr(configFile.size() - 5) != ".conf")
		throw(std::runtime_error("Provided config file '" + configFile + "' doesn't have a .conf extension."));
	fileCheck = stat(configFile.c_str(), &fileCheckBuff);
	if (fileCheck != 0 || !(fileCheckBuff.st_mode & S_IFREG))
		throw(std::runtime_error("Provided config file '" + configFile + "' is an invalid file."));
	file.open(configFile.c_str());
	if (!file)
		throw(std::runtime_error("Provided config file '" + configFile + "' is not accessible."));
	if (!(file >> c)) // check if the file is empty by trying to read a character from it
		throw(std::runtime_error("Provided config file '" + configFile + "' is empty."));
	file.putback(c); //	putting the character back bcs it will be read again later
	tmpFileContent << file.rdbuf();
	this->_configContent = tmpFileContent.str();

	std::cout << "CONTENT (initial)" << std::endl;
	std::cout << this->_configContent << std::endl;
	removeCommentsAndEmptyLines();
	
	detectServerBlocks();
	std::cout << "DETECTED SERVER BLOCKS" << std::endl;
	printServerBlocks();
	file.close();
}

/**
 * @warning Incomplete
*/
ServerConfig::ServerConfig(const ServerConfig& copy)
{
	(void) copy;
}

/**
 * @warning Incomplete
*/
ServerConfig	&ServerConfig::operator = (const ServerConfig &src)
{
	(void) src;
	return (*this);
}

ServerConfig::~ServerConfig(void)
{
	return ;
}

std::string	ServerConfig::getFileContent(void) const
{
	return (this->_configContent);
}

void	ServerConfig::removeCommentsAndEmptyLines(void)
{
	size_t				start;
	size_t				end;
	std::string			line;
	std::stringstream	ss;
	std::string			newFileContent;

	start = this->_configContent.find('#');
	while (start != std::string::npos)
	{
		end = this->_configContent.find('\n', start);
		this->_configContent.erase(start, end - start);
		start = this->_configContent.find('#');
	}
	std::cout << "CONTENT (removed comments)" << std::endl;
	std::cout << this->_configContent << std::endl;
	ss.str(this->_configContent);
	while (std::getline(ss, line))
	{
		start = line.find_first_not_of("\t\n\v\f\r ");
		end = line.find_last_not_of("\t\n\v\f\r ");
		if (start != std::string::npos && end != std::string::npos) // trim whitespaces from the end of the line
			line = line.substr(start, end - start + 1);
		else
			line = "";
		if (!line.empty()) // if line not empty (after removing the start and end whitespaces) add to newFileContent
			newFileContent += line + '\n';
	}
	this->_configContent = newFileContent;
	std::cout << "CONTENT (removed empty lines)" << std::endl;
	std::cout << this->_configContent << std::endl;
}


size_t	validateServerBlockStart(size_t pos, std::string &configContent)
{
	size_t	i;

	i = configContent.find_first_not_of(" \t\n\r", pos);
	if (i == std::string::npos)
		return (pos);
	if (configContent.substr(i, 6) != "server")
		throw(std::runtime_error("Config parser: Invalid start of the server scope."));
	i += 6;
	i = configContent.find_first_not_of(" \t\n\r", i);
	if (i == std::string::npos || configContent[i] != '{')
		throw(std::runtime_error("Config parser: Invalid start of the scope."));
	return (i);
}

size_t	validateServerBlockEnd(size_t pos, std::string &configContent)
{
	size_t	i;
	size_t	nested;

	nested = 0;
	i = pos;
	while (i != std::string::npos)
	{
		i = configContent.find_first_of("{}", i);
		if (i != std::string::npos)
		{
			if (configContent[i] == '{')
				nested++;
			else if (nested == 0)
				return (i);
			else
				nested--;
			i++;
		}
	}
	return (pos);
}


void	ServerConfig::detectServerBlocks(void)
{
	size_t		serverStart;
	size_t		serverEnd;
	std::string	serverBlock;

	if (this->_configContent.find("server") == std::string::npos)
		throw(std::runtime_error("Config parser: No server block found."));
	serverStart = validateServerBlockStart(0, this->_configContent);
	serverEnd = validateServerBlockEnd(serverStart + 1, this->_configContent);
	if (serverEnd == std::string::npos)
		throw(std::runtime_error("Config parser: Server block has no scope."));
	while (serverStart < this->_configContent.length() - 1 && serverEnd != serverStart)
	{
		if (serverStart == serverEnd)
			throw(std::runtime_error("Config parser: Server block has no scope."));
		serverBlock = this->_configContent.substr(serverStart, serverEnd - serverStart + 1);
		this->_serverBlocks.push_back(serverBlock);
		serverStart = validateServerBlockStart(serverEnd + 1, this->_configContent);
		serverEnd = validateServerBlockEnd(serverStart + 1, this->_configContent);
	}
}

void	ServerConfig::printServerBlocks(void) const
{
	for (size_t i = 0; i < this->_serverBlocks.size(); i++)
	{
		std::cout << "Server block " << i << ": " << std::endl;
		std::cout << this->_serverBlocks[i] << std::endl;
	}
}
