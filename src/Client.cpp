/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/15 11:11:16 by aulicna           #+#    #+#             */
/*   Updated: 2024/07/24 15:20:55 by plouda           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Client.hpp"

Client::Client(void): bufferUnchecked(false),  _requestID(0), _clientSocket(-1), 
	_timeLastMessage(time(NULL)), _timeLastValidHeaderEnd(time(NULL)),
	_receivedData(), _receivedHeader(), _portConnectedOn(0)
{
	memset(&this->_clientAddr, 0, sizeof(this->_clientAddr));
	return ;
}

Client::Client(const Client& copy)
{
	this->_clientSocket = copy._clientSocket;
	this->_timeLastMessage = copy._timeLastMessage;
	this->_timeLastValidHeaderEnd = copy._timeLastValidHeaderEnd;
	this->_receivedData = copy._receivedData;
	this->_receivedHeader = copy._receivedHeader;
	this->_portConnectedOn = copy._portConnectedOn;
	this->_serverConfig = copy._serverConfig;
	this->request = copy.request;
	this->bufferUnchecked = copy.bufferUnchecked;
	this->_clientAddr = copy._clientAddr;
	this->_requestID = copy._requestID;
}

Client	&Client::operator = (const Client &src)
{
	if (this != &src)
	{
		this->_clientSocket = src._clientSocket;
		this->_timeLastMessage = src._timeLastMessage;
		this->_timeLastValidHeaderEnd = src._timeLastValidHeaderEnd;
		this->_receivedData = src._receivedData;
		this->_receivedHeader = src._receivedHeader;
		this->_portConnectedOn = src._portConnectedOn;
		this->_serverConfig = src._serverConfig;
		this->request = src.request;
		this->bufferUnchecked = src.bufferUnchecked;
		this->_clientAddr = src._clientAddr;
		this->_requestID = src._requestID;
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

void	Client::updateTimeLastValidHeaderEnd(void)
{
	this->_timeLastValidHeaderEnd = time(NULL);
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

void	Client::setClientAddr(struct sockaddr_in clientAddr)
{
	this->_clientAddr = clientAddr;
}

int	Client::getClientSocket(void) const
{
	return (this->_clientSocket);
}

time_t	Client::getTimeLastMessage(void) const
{
	return (this->_timeLastMessage);
}

time_t	Client::getTimeLastValidHeaderEnd(void) const
{
	return (this->_timeLastValidHeaderEnd);
}

const octets_t	&Client::getReceivedData(void) const
{
	return (this->_receivedData);
}

octets_t	Client::getReceivedHeader(void) const
{
	return (this->_receivedHeader);
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

const struct sockaddr_in	&Client::getClientAddr(void) const
{
	return (this->_clientAddr);
}

const int &Client::getRequestID(void) const
{
	return (this->_requestID);
}

void Client::printReceivedData(void) const
{
	for (octets_t::const_iterator it = this->_receivedData.begin(); it != this->_receivedData.end(); it++)
		std::cout << static_cast<char>(*it);
	std::cout << std::endl;
}

void		Client::printReceivedHeader(void) const
{
	for (octets_t::const_iterator it = this->_receivedHeader.begin(); it != this->_receivedHeader.end(); it++)
		std::cout << static_cast<char>(*it);
	std::cout << std::endl;
}
void	Client::clearReceivedData(void)
{
	this->_receivedData.clear();
}

void	Client::eraseRangeReceivedData(size_t start, size_t end)
{
	if (start <= end && end <= this->_receivedData.size())
		this->_receivedData.erase(this->_receivedData.begin() + start, this->_receivedData.begin() + end);
}

void	Client::clearReceivedHeader(void)
{
	this->_receivedHeader.clear();
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

bool Client::hasValidHeaderEnd(void)
{
	std::string sequences[] = {"\r\n\n", "\n\n", "\n\r\n", "\r\n\r\n"};
	std::vector<unsigned char>::const_iterator endOfSequence;

	for (int i = 0; i < 4; ++i)
	{
		endOfSequence = std::search(this->_receivedData.begin(), this->_receivedData.end(), sequences[i].begin(), sequences[i].end());
		if (endOfSequence != this->_receivedData.end())
		{
			this->updateTimeLastValidHeaderEnd();
			return (true);
		}
	}
	if (this->_receivedData.size() > CLIENT_MESSAGE_BUFF * 2)
		throw(ResponseException(413, "Request header too large"));
	return (false);
}

void Client::separateValidHeader(void)
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
// we can be sure that the sequence will be found since this function is called only once hasValidHeaderEnd returns true
//	if (endOfSequence == this->_receivedData.end())
//		return (false);
	this->_receivedHeader.insert(this->_receivedHeader.end(), this->_receivedData.begin(), sequenceEnd);
	this->_receivedData.erase(this->_receivedData.begin(), sequenceEnd);
}

void	Client::incrementRequestID(void)
{
	if (this->_requestID < INT_MAX)
		this->_requestID++;
	else
		this->_requestID = 1;
}
