/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/14 19:17:59 by aulicna           #+#    #+#             */
/*   Updated: 2024/05/22 14:53:58 by aulicna          ###   ########.fr       */
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
		void	updateTimeLastMessage(void);
		void	updateReceivedData(uint8_t *recvBuf, ssize_t &bytesReceived);

		int				getClientSocket(void) const;
		time_t			getTimeLastMessage(void) const;
		const octets_t	&getReceivedData(void) const;

		void		printReceivedData(void) const;
		void		printDataToParse(void) const;
		void		clearReceivedData(void);
		void		clearDataToParse(void);
		void		trimHeaderEmptyLines(void);
		bool		findValidHeaderEnd(void);

	private:
		int					_clientSocket;
		time_t				_timeLastMessage;
		octets_t			_receivedData;
		octets_t			_dataToParse;

};

#endif
