/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/14 19:17:59 by aulicna           #+#    #+#             */
/*   Updated: 2024/06/11 11:32:11 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "webserv.hpp"
# include "ServerConfig.hpp"
# include "HttpRequest.hpp"

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
		void	setPortConnectedOn(unsigned short portConnectedOn);
		void	setServerConfig(const ServerConfig &serverConfig);

		int					getClientSocket(void) const;
		time_t				getTimeLastMessage(void) const;
		const octets_t		&getReceivedData(void) const;
		octets_t		getDataToParse(void) const;
		unsigned short		getPortConnectedOn(void) const;
		const ServerConfig	&getServerConfig(void) const;
		const HttpRequest	&getRequest(void) const;

		void		printReceivedData(void) const;
		void		printDataToParse(void) const;
		void		clearReceivedData(void);
		void		eraseRangeReceivedData(size_t start, size_t end);
		void		clearDataToParse(void);
		void		trimHeaderEmptyLines(void);
		bool		findValidHeaderEnd(void);
		
		HttpRequest	request;

	private:
		int					_clientSocket;
		time_t				_timeLastMessage;
		octets_t			_receivedData;
		octets_t			_dataToParse;
		unsigned short		_portConnectedOn;
		ServerConfig		_serverConfig;

};

#endif
