/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/22 18:27:49 by aulicna           #+#    #+#             */
/*   Updated: 2024/05/22 19:59:06 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"

ServerConfig::ServerConfig(void)
{
	return ;
}

ServerConfig::ServerConfig(std::string configFile)
{
	std::ifstream	file;
	int				fileCheck;
	struct stat		fileCheckBuff;
	char			c;
	std::string		line;
	std::stringstream	tmpFileContent;
	std::string			fileContent;

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
	fileContent = tmpFileContent.str();
	std::cout << fileContent << std::endl;
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
