/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/18 15:03:51 by plouda            #+#    #+#             */
/*   Updated: 2024/07/22 15:49:04 by plouda           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_HPP
#define LOGGER_HPP
#include "webserv.hpp"
#include <ctime>
# define RED "\e[38;5;160m"
# define ORANGE "\e[38;5;214m"
# define YELLOW "\e[38;5;226m"
# define GREY "\e[38;5;250m"
# define GREEN "\e[38;5;113m"
# define MAGENTA "\e[38;5;201m"
# define CYAN "\e[38;5;51m"
# define BLUE "\e[38;5;33m"
# define LOG_BUF 8192

enum LogLevel
{
	DEBUG,
	INFO,
	NOTICE,
	WARNING,
	ERROR,
	DISABLED
};

// enum DebugCategory
// {
// 	REQUEST,
// 	RESPONSE,
// 	CGI,
// 	CONFIG
// };

class Logger
{
	private:
		static const LogLevel	_logLevel;
		static std::string			_logBuffer;
		static int					_outputFd;
		static std::map<ServerSection, bool>	_logOptions;
		static std::map<ServerSection, bool>	initOptions();

	public:
		static bool	readyToWrite;

		Logger();
		Logger(const Logger& refObj);
		~Logger();
		static void			log(LogLevel level, ServerSection cat, std::string message, std::string details);
		static void			safeLog(LogLevel level, ServerSection cat, std::string message, std::string details);
		static std::string&	getLogBuffer();
		static int&			getOutputFd();
		static void			eraseLogRange(size_t toErase);
};

typedef void(Logger::*RaiseMessage)(void);


#endif  // LOGGER_HPP
