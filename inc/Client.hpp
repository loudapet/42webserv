/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/14 19:17:59 by aulicna           #+#    #+#             */
/*   Updated: 2024/07/24 15:12:40 by plouda           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "webserv.hpp"
# include "ServerConfig.hpp"
# include "HttpRequest.hpp"
# include "ResponseException.hpp"

class Client
{
	public:
		Client(void);
		Client(const Client& copy);
		Client	&operator=(const Client &src);
		~Client(void);

		void	setClientSocket(int clientSocket);
		void	updateTimeLastMessage(void);
		void	updateTimeLastValidHeaderEnd(void);
		void	updateReceivedData(uint8_t *recvBuf, ssize_t &bytesReceived);
		void	setPortConnectedOn(unsigned short portConnectedOn);
		void	setServerConfig(const ServerConfig &serverConfig);
		void	setClientAddr(struct sockaddr_in clientAddr);

		int							getClientSocket(void) const;
		time_t						getTimeLastMessage(void) const;
		time_t						getTimeLastValidHeaderEnd(void) const;
		const octets_t				&getReceivedData(void) const;
		octets_t					getReceivedHeader(void) const;
		unsigned short				getPortConnectedOn(void) const;
		const ServerConfig			&getServerConfig(void) const;
		const HttpRequest			&getRequest(void) const;
		const struct sockaddr_in	&getClientAddr(void) const;
		const int&					getRequestID(void) const;

		void		printReceivedData(void) const;
		void		printReceivedHeader(void) const;
		void		clearReceivedData(void);
		void		eraseRangeReceivedData(size_t start, size_t end);
		void		clearReceivedHeader(void);
		void		trimHeaderEmptyLines(void);
		bool		hasValidHeaderEnd(void);
		void		separateValidHeader(void);
		void		incrementRequestID(void);
		
		HttpRequest	request;
		bool		bufferUnchecked;

	private:
		int					_requestID;
		int					_clientSocket;
		time_t				_timeLastMessage;
		time_t				_timeLastValidHeaderEnd;
		octets_t			_receivedData;
		octets_t			_receivedHeader;
		unsigned short		_portConnectedOn;
		ServerConfig		_serverConfig;
		struct sockaddr_in	_clientAddr;
};

#endif
