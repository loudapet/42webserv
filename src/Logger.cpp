/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/18 15:03:44 by plouda            #+#    #+#             */
/*   Updated: 2024/07/19 17:15:37 by plouda           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Logger.hpp"

const enum LogLevel Logger::_logLevel = L_DEBUG;
std::string	Logger::_logBuffer = "";
int Logger::_outputFd = STDOUT_FILENO;
bool Logger::readyToWrite = false;

Logger::Logger()
{
	return;
}

Logger::Logger(const Logger& refObj)
{
	*this = refObj;
}

Logger::~Logger()
{
	return;
}

void	Logger::log(enum LogLevel level, std::string message, std::string details)
{
	std::string		levelArray[5] = {"DEBUG", "INFO", "NOTICE", "WARNING", "ERROR"};
	std::string		clrArray[5] = {GREY, GREEN, YELLOW, ORANGE, RED};
	time_t			curr_time;
	tm				*curr_tm;
	char			buffer[100];

	std::time(&curr_time);
	curr_tm = std::gmtime(&curr_time);
	std::strftime(buffer, 100, "[%m-%d-%y %H:%M:%S]", curr_tm);
	if (level >= Logger::_logLevel)
		std::cout << clrArray[level] << buffer <<  " <" << levelArray[level] << "> " << message << details << RESET << std::endl;
	// std::cout << GREY << "HELLO WORLD QWRETEUOLOIISPOAJKXHBVCHJVD" << RESET << std::endl;
	// std::cout << GREEN << "HELLO WORLD QWRETEUOLOIISPOAJKXHBVCHJVD" << RESET << std::endl;
	// std::cout << YELLOW << "HELLO WORLD QWRETEUOLOIISPOAJKXHBVCHJVD" << RESET << std::endl;
	// std::cout << ORANGE << "HELLO WORLD QWRETEUOLOIISPOAJKXHBVCHJVD" << RESET << std::endl;
	// std::cout << RED << "HELLO WORLD QWRETEUOLOIISPOAJKXHBVCHJVD" << RESET << std::endl;
}

void	Logger::safeLog(enum LogLevel level, std::string message, std::string details)
{
	if (level >= Logger::_logLevel)
	{
		std::string		levelArray[5] = {"DEBUG", "INFO", "NOTICE", "WARNING", "ERROR"};
		std::string		clrArray[5] = {GREY, GREEN, YELLOW, ORANGE, RED};
		time_t			curr_time;
		tm				*curr_tm;
		char			buffer[100];
		std::string		logLine;

		std::time(&curr_time);
		curr_tm = std::gmtime(&curr_time);
		std::strftime(buffer, 100, "[%m-%d-%y %H:%M:%S]", curr_tm);
		logLine = clrArray[level] + buffer + " <" + levelArray[level] + "> " + message + details + RESET + '\n';
		//std::cout << clrArray[level] << buffer <<  " <" << levelArray[level] << "> " << message << details << RESET << std::endl;
		Logger::_logBuffer.append(logLine);
		if (!readyToWrite)
			readyToWrite = true;
	}
}

void	Logger::eraseLogRange(size_t toErase)
{
	if (_logBuffer.size() >= toErase)
		_logBuffer.erase(0, toErase);
}

std::string&	Logger::getLogBuffer()
{
	return (_logBuffer);
}

int&	Logger::getOutputFd()
{
	return (_outputFd);
}
