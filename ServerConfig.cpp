/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/29 12:19:56 by aulicna           #+#    #+#             */
/*   Updated: 2024/05/29 14:55:26 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "ServerConfig.hpp"

ServerConfig::ServerConfig(void)
{
	return ;
}

std::vector<std::string>	splitServerBlock(std::string &serverBlock)
{
	std::vector<std::string>	serverBlockElements;
	size_t						start;
	size_t						end;

	start = 0;
	end = serverBlock.find_first_of("\n ");
	while (end != std::string::npos)
	{
		serverBlockElements.push_back(serverBlock.substr(start, end - start + 1));
		start = end + 1;
		end = serverBlock.find_first_of("\n ", start);
	}
	return (serverBlockElements);
}

ServerConfig::ServerConfig(std::string &serverBlock)
{
	std::vector<std::string> serverBlockElements;

	serverBlockElements = splitServerBlock(serverBlock);
	std::cout << "elements: " << std::endl;	
	for (size_t i = 0; i < serverBlockElements.size(); i++)
		std::cout << serverBlockElements[i] << std::endl;
}

ServerConfig::ServerConfig(const ServerConfig& copy)
{
	(void)copy;
}

ServerConfig	&ServerConfig::operator = (const ServerConfig &src)
{
	(void)src;
	return (*this);
}

ServerConfig::~ServerConfig(void)
{
	return ;
}
