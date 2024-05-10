#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

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
	serverAddr.sin_port = htons(8000); // Port number of the server
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // IP address of the server

	// Connect to the server
	if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
	{
		std::cerr << "Error: Connection failed\n";
		close(clientSocket);
		return (1);
	}

	// Send data to the server
	const char *message = "Hello, server!";
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
