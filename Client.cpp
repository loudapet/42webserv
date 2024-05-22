/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/15 11:11:16 by aulicna           #+#    #+#             */
/*   Updated: 2024/05/21 15:41:16 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(void): _timeLastMessage(time(NULL)), _receivedData(), _dataToParse()
{
	return ;
}

Client::Client(const Client& copy)
{
	this->_clientSocket = copy._clientSocket;
	this->_timeLastMessage = copy._timeLastMessage;
	this->_receivedData = copy._receivedData;
	this->_dataToParse = copy._dataToParse;
}

Client	&Client::operator = (const Client &src)
{
	if (this != &src)
	{
		this->_clientSocket = src._clientSocket;
		this->_timeLastMessage = src._timeLastMessage;
		this->_receivedData = src._receivedData;
		this->_dataToParse = src._dataToParse;
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

int	Client::getClientSocket(void) const
{
	return (this->_clientSocket);
}

void	Client::updateTimeLastMessage(void)
{
	this->_timeLastMessage = time(NULL);
}

time_t	Client::getTimeLastMessage(void) const
{
	return (this->_timeLastMessage);
}

void	Client::updateReceivedData(uint8_t *recvBuf, ssize_t &bytesReceived)
{
	this->_receivedData.insert(this->_receivedData.end(), recvBuf, recvBuf + bytesReceived);
}

const octets_t	&Client::getReceivedData(void) const
{
	return (this->_receivedData);
}

void		Client::printReceivedData(void) const
{
	for (octets_t::const_iterator it = this->_receivedData.begin(); it != this->_receivedData.end(); it++)
		std::cout << static_cast<char>(*it);
	std::cout << std::endl;
}

void		Client::printDataToParse(void) const
{
	for (octets_t::const_iterator it = this->_dataToParse.begin(); it != this->_dataToParse.end(); it++)
		std::cout << static_cast<char>(*it);
	std::cout << std::endl;
}
void	Client::clearReceivedData(void)
{
	this->_receivedData.clear();
}

void	Client::clearDataToParse(void)
{
	this->_dataToParse.clear();
}

void	Client::trimHeaderEmptyLines(void)
{
	octets_t::iterator	nl = std::find(this->_receivedData.begin(), this->_receivedData.end(), '\n');
	octets_t 			line(this->_receivedData.begin(), nl);
	while (this->_receivedData.size() > 0 && (line.size() == 0 || (line.size() == 1 && line[0] == '\r'))
			&& nl != this->_receivedData.end())
	{
		this->_receivedData.erase(this->_receivedData.begin(), nl + 1);
		nl = std::find(this->_receivedData.begin(), this->_receivedData.end(), '\n');
		line = octets_t(this->_receivedData.begin(), nl);
	}
}

bool Client::findValidHeaderEnd(void)
{
	std::string sequences[] = {"\r\n\n", "\n\n", "\n\r\n", "\r\n\r\n"};
	std::vector<unsigned char>::iterator endOfSequence;
	std::vector<unsigned char>::iterator sequenceEnd;

	for (int i = 0; i < 4; ++i)
	{
		endOfSequence = std::search(this->_receivedData.begin(), this->_receivedData.end(), sequences[i].begin(), sequences[i].end());
		if (endOfSequence != this->_receivedData.end())
		{
			sequenceEnd = endOfSequence + sequences[i].size();
			break;
		}
	}
	if (endOfSequence == this->_receivedData.end())
		return (false);
	this->_dataToParse.insert(this->_dataToParse.end(), this->_receivedData.begin(), sequenceEnd);
	this->_receivedData.erase(this->_receivedData.begin(), sequenceEnd);
	return (true);
}
