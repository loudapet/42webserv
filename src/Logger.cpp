/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/18 15:03:44 by plouda            #+#    #+#             */
/*   Updated: 2024/07/18 16:00:25 by plouda           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Logger.hpp"

Logger::Logger()
{
	return;
}

Logger::Logger(const Logger& refObj)
{
	*this = refObj;
}

Logger& Logger::operator = (const Logger& refObj)
{
	if (this != &refObj)
		// copy values over
	return (*this);
}

Logger::~Logger()
{
	return;
}

void	Logger::log(enum LogLevel level, std::string message)
{
	// std::string		levelArray[4] = {"DEBUG", "INFO", "WARNING", "ERROR"};
	// RaiseMessage	fptrArray[4] = {&Logger::debug, &Logger::info,
	// 								&Logger::warning, &Logger::error};

	// for (int i = 0 ; i < 4 ; i++)
	// 	if (levelArray[i] == level)
	// 		(this->*fptrArray[i])();
}