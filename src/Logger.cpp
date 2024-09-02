/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/18 15:03:44 by plouda            #+#    #+#             */
/*   Updated: 2024/09/02 10:02:44 by plouda           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Logger.hpp"
#include "../inc/ServerMaster.hpp"

enum LogLevel Logger::_logLevel = INFO;
std::string			Logger::_logBuffer = "";
const std::vector<std::string> 	Logger::_levelArray = initLogLevels();
const std::string 	Logger::_clrArray[5] = {GREY, GREEN, YELLOW, ORANGE, RED};
int					Logger::_outputFd = STDERR_FILENO;
bool				Logger::readyToWrite = false;
int					Logger::_activeClient = 0;
int					Logger::_activeRequestID = 0;
std::map<int,int> 	Logger::_fdToClientID = std::map<int,int>();
std::map<enum ServerSection, bool>	Logger::_logOptions = initOptions();

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

std::string	Logger::getCurrentLogTime(void)
{
	time_t			curr_time;
	tm				*curr_tm;
	char			buffer[100];

	std::time(&curr_time);
	curr_tm = std::gmtime(&curr_time);
	std::strftime(buffer, 100, "[%m-%d-%y %H:%M:%S]", curr_tm);
	return (buffer);
}

std::map<enum ServerSection, bool>	Logger::initOptions()
{
	std::map<enum ServerSection, bool> tmp;
	tmp[REQUEST] = true;
	tmp[RESPONSE] = true;
	tmp[CGI] = true;
	tmp[CONFIG] = true;
	tmp[SERVER] = true;
	return (tmp);
}

std::vector<std::string>	Logger::initLogLevels()
{
	std::vector<std::string> tmp = std::vector<std::string>(5);
	tmp[0] = "DEBUG";
	tmp[1] = "INFO";
	tmp[2] = "NOTICE";
	tmp[3] = "WARNING";
	tmp[4] = "ERROR";
	return (tmp);
}

void	Logger::log(enum LogLevel level, enum ServerSection cat, std::string message, std::string details)
{
	if (level >= Logger::_logLevel)
	{
		if (level != DEBUG || (level == DEBUG && _logOptions[cat]))
		{
			std::string	logLine;
			logLine = Logger::getCurrentLogTime() + " <" + _levelArray[level] + "> " + message + details;
			if (Logger::_outputFd == STDERR_FILENO)
				std::cerr << _clrArray[level] << logLine << RESET << std::endl;
			else
				write(Logger::_outputFd, (logLine + '\n').c_str(), logLine.size() + 1);
		}
	}
}

void	Logger::safeLog(enum LogLevel level, enum ServerSection cat, std::string message, std::string details)
{
	if (level >= Logger::_logLevel)
	{
		if (level != DEBUG || (level == DEBUG && _logOptions[cat]))
		{
			std::string	logLine;
			std::map<int,int>::iterator activeID = Logger::_fdToClientID.find(Logger::_activeClient);

			if ((cat != SERVER && cat != CONFIG) && activeID != Logger::_fdToClientID.end())
				logLine = Logger::getCurrentLogTime() + " <" + _levelArray[level] + "> [C_ID:" + itoa(activeID->second) + "][" + itoa(Logger::_activeRequestID) + "] "+ message + details;
			else
				logLine = Logger::getCurrentLogTime() + " <" + _levelArray[level] + "> " + message + details;
			if (Logger::_outputFd == STDERR_FILENO)
				Logger::_logBuffer.append(_clrArray[level] + logLine + RESET + '\n');
			else
				Logger::_logBuffer.append(logLine + '\n');
			if (!readyToWrite)
				readyToWrite = true;
		}
	}
}

void	Logger::eraseLogRange(size_t toErase)
{
	if (_logBuffer.size() >= toErase)
		_logBuffer.erase(0, toErase);
}

void	Logger::mapFdToClientID(int fd)
{
	if (Logger::_fdToClientID.insert(std::make_pair(fd, ServerMaster::getConnectionCounter())).second == false)
		Logger::_fdToClientID[fd] = ServerMaster::getConnectionCounter();
}

void Logger::setActiveClient(int fd)
{
	Logger::_activeClient = fd;
}

void Logger::setActiveRequestID(int requestID)
{
	Logger::_activeRequestID = requestID;
}

std::string &Logger::getLogBuffer()
{
	return (_logBuffer);
}

int&	Logger::getOutputFd()
{
	return (_outputFd);
}

const std::vector<std::string>	&Logger::getLevelArray()
{
	return (_levelArray);
}

void	Logger::setOutputFd(int fd)
{
	_outputFd = fd;
}

void Logger::setLogLevel(LogLevel level)
{
	_logLevel = level;
}
