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
	this->_fileContent = tmpFileContent.str();

	std::cout << "CONTENT (initial)" << std::endl;
	std::cout << "First:" << this->_fileContent[0] << "|" << std::endl;
	std::cout << this->_fileContent << std::endl;
	removeCommentsAndEndSpaces();
	std::cout << "CONTENT (removed comments)" << std::endl;
	std::cout << this->_fileContent << std::endl;
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
	return (this->_fileContent);
}

void	ServerConfig::removeCommentsAndEndSpaces(void)
{
	size_t	commentStart;
	size_t	commentEnd;
	size_t	pos;

	commentStart = this->_fileContent.find('#');
	while (commentStart != std::string::npos)
	{
		commentEnd = this->_fileContent.find('\n', commentStart);
		this->_fileContent.erase(commentStart, commentEnd - commentStart);
		commentStart = this->_fileContent.find('#');
	}
	pos = 0;
	pos = this->_fileContent.size() - 1;
	while (this->_fileContent[pos] && isspace(this->_fileContent[pos]))
		pos--;
	this->_fileContent = this->_fileContent.substr(0, pos + 1);
}