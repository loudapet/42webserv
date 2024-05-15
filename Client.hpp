/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/14 19:17:59 by aulicna           #+#    #+#             */
/*   Updated: 2024/05/15 17:28:22 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "webserv.hpp"

class Client
{
	public:
		Client(void);
		Client(const Client& copy);
		Client	&operator=(const Client &src);
		~Client(void);

		void	setClientSocket(int clientSocket);
		void	updateTimeLastMessage();

		int		getClientSocket(void) const;
		time_t	getTimeLastMessage(void) const;

	private:
		int					_clientSocket;
		time_t				_timeLastMessage;
};

#endif
