#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include "webserv.hpp"

//"\n\r\n\n\r\nGET /index.html HTTP/1.1\r\nHost: www.example.com\r\nUser-Agent: Mozilla/5.0\nAccept: text/html, */*\r\nAccept-Language: en-us\nAccept-Charset: ISO-8859-1,utf-8\nConnection: keep-alive\r\n\n";
//GET / HTTP/1.1
//Host: www.example.com
//User-Agent: Mozilla/5.0
//Accept: text/html, */*
//Accept-Language: en-us
//Accept-Charset: ISO-8859-1,utf-8\n
//Connection: keep-alive

int main(void)
{
	// Create a socket
	int clientSocket;
	
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == -1)
	{
		std::cerr << "Error: Failed to create socket\n";
		return (1);
	}

	// Server address
	struct sockaddr_in serverAddr;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8002); // Port number of the server
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // IP address of the server

	// Connect to the server
	if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
	{
		std::cerr << "Error: Connection failed\n";
		close(clientSocket);
		return (1);
	}
	// Send data to the server
	//const char *message = "\nGET / HTTP/1.0\nHost: example.com\nConnection: close\n\n";
	const char *message = "GET /index.html?q=key#now HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
	if (send(clientSocket, message, strlen(message), 0) == -1)
	{
		std::cerr << "Error: Failed to send data\n";
		close(clientSocket);
		return (1);
	}

	// Receive response from the server
	char buffer[1024] = {0};
	if (recv(clientSocket, buffer, 1024, 0) == -1)
	{
		std::cerr << "Error: Failed to receive data\n";
		close(clientSocket);
		return (1);
	}
	std::cout << "Server response: " << buffer << std::endl;

	// Close the socket
	close(clientSocket);
	return (0);
}


