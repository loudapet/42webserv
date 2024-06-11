/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/15 11:11:16 by aulicna           #+#    #+#             */
/*   Updated: 2024/06/11 14:51:18 by plouda           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Client.hpp"

Client::Client(void): _clientSocket(-1), _timeLastMessage(time(NULL)), _receivedData(), _dataToParse(), _portConnectedOn(0)
{
	return ;
}

Client::Client(const Client& copy)
{
	this->_clientSocket = copy._clientSocket;
	this->_timeLastMessage = copy._timeLastMessage;
	this->_receivedData = copy._receivedData;
	this->_dataToParse = copy._dataToParse;
	this->_portConnectedOn = copy._portConnectedOn;
	this->_serverConfig = copy._serverConfig;
}

Client	&Client::operator = (const Client &src)
{
	if (this != &src)
	{
		this->_clientSocket = src._clientSocket;
		this->_timeLastMessage = src._timeLastMessage;
		this->_receivedData = src._receivedData;
		this->_dataToParse = src._dataToParse;
		this->_portConnectedOn = src._portConnectedOn;
		this->_serverConfig = src._serverConfig;
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

void	Client::updateTimeLastMessage(void)
{
	this->_timeLastMessage = time(NULL);
}

void	Client::updateReceivedData(uint8_t *recvBuf, ssize_t &bytesReceived)
{
	this->_receivedData.insert(this->_receivedData.end(), recvBuf, recvBuf + bytesReceived);
}

void	Client::setPortConnectedOn(unsigned short portConnectedOn)
{
	this->_portConnectedOn = portConnectedOn;
}

void	Client::setServerConfig(const ServerConfig &serverConfig)
{
	this->_serverConfig = serverConfig;
}

int	Client::getClientSocket(void) const
{
	return (this->_clientSocket);
}

time_t	Client::getTimeLastMessage(void) const
{
	return (this->_timeLastMessage);
}

const octets_t	&Client::getReceivedData(void) const
{
	return (this->_receivedData);
}

octets_t	Client::getDataToParse(void) const
{
	return (this->_dataToParse);
}

unsigned short	Client::getPortConnectedOn(void) const
{
	return (this->_portConnectedOn);
}

const ServerConfig	&Client::getServerConfig(void) const
{
	return (this->_serverConfig);
}
	
const HttpRequest	&Client::getRequest(void) const
{
	return (this->request);
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

void	Client::eraseRangeReceivedData(size_t start, size_t end)
{
	std::cout << "RECEIVED DATA " << this->_receivedData.size() << std::endl;
	if (start <= end && start >= 0 && end <= this->_receivedData.size())
	{
		std::cout << "Show data to be deleted: " << std::endl;
		std::cout << this->_receivedData << std::endl;
		this->_receivedData.erase(this->_receivedData.begin() + start, this->_receivedData.begin() + end);
	}
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
