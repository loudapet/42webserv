/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okraus <okraus@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/18 15:03:51 by plouda            #+#    #+#             */
/*   Updated: 2024/07/22 10:57:38 by okraus           ###   ########.fr       */
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
	L_DEBUG,
	INFO,
	NOTICE,
	WARNING,
	ERROR,
	DISABLED
};

class Logger
{
	private:
		static const enum LogLevel	_logLevel;
		static std::string			_logBuffer;
		static int					_outputFd;

	public:
		static bool	readyToWrite;

		Logger();
		Logger(const Logger& refObj);
		~Logger();
		static void			log(enum LogLevel level, std::string message, std::string details);
		static void			safeLog(enum LogLevel level, std::string message, std::string details);
		static std::string&	getLogBuffer();
		static int&			getOutputFd();
		static void			eraseLogRange(size_t toErase);
};

typedef void(Logger::*RaiseMessage)(void);


#endif  // LOGGER_HPP
