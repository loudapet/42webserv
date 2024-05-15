/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/15 11:11:16 by aulicna           #+#    #+#             */
/*   Updated: 2024/05/15 17:31:11 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(void)
{
	this->_timeLastMessage = time(NULL);
}

Client::Client(const Client& copy)
{
	this->_clientSocket = copy._clientSocket;
	this->_timeLastMessage = copy._timeLastMessage;
}

Client	&Client::operator = (const Client &src)
{
	if (this != &src)
	{
		this->_clientSocket = src._clientSocket;
		this->_timeLastMessage = src._timeLastMessage;
	}
	return (*this);
}

Client::~Client(void)
{
	return ;
}

void	Client::setClientSocket(int clientSocket)
{
	this->_clientSocket = clientSocket;
}

void	Client::updateTimeLastMessage()
{
	this->_timeLastMessage = time(NULL);
}

int	Client::getClientSocket(void) const
{
	return (this->_clientSocket);
}

time_t	Client::getTimeLastMessage(void) const
{
	return (this->_timeLastMessage);
}
