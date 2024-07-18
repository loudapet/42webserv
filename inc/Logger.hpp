/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/18 15:03:51 by plouda            #+#    #+#             */
/*   Updated: 2024/07/18 16:07:18 by plouda           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_HPP
#define LOGGER_HPP
#include "webserv.hpp"

enum LogLevel
{
	L_DEBUG,
	INFO,
	WARNING,
	ERROR
};

class Logger
{
	private:
		static void	log(enum LogLevel level, std::string message);

	public:
		Logger();
		Logger(const Logger& refObj);
		Logger& operator = (const Logger& refObj);
		~Logger();
};

typedef void(Logger::*RaiseMessage)(void);


#endif  // LOGGER_HPP
