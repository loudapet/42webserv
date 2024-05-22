/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/22 18:28:00 by aulicna           #+#    #+#             */
/*   Updated: 2024/05/22 18:31:48 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include "webserv.hpp"

class ServerConfig
{
	public:
		ServerConfig(std::string configFile);
		ServerConfig(const ServerConfig& copy);
		ServerConfig	&operator=(const ServerConfig &src);
		~ServerConfig(void);
	
	private:
		ServerConfig(void);

};

#endif
