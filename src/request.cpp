/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/07 10:46:07 by plouda            #+#    #+#             */
/*   Updated: 2024/06/10 10:07:37 by plouda           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <vector>
#include <string>
#include "../inc/HttpRequest.hpp"

octets_t	convertStringToOctets(std::string& str)
{
	octets_t vec(str.begin(), str.end());
	return (vec);
}

int	main()
{

	//std::string	startLine("\n\r\n\n\r\nGET http://%E2%82%AC127.0.0.1%21%22o:70/%E2%82%ACe/wher%22//./eyo?q=now%34?key=value/index.ttx#%32 HTTP/1.1\r\n"); // if testing for 0-byte, pass precise length to constructor
	//std::string	startLine("\n\r\n\n\r\nGET /./ HTTP/1.1\r\n"); // if testing for 0-byte, pass precise length to constructor
	//std::string	startLine("\n\r\n\n\r\nGET /./././a/b//////.. HTTP/1.1\r\n");
	//std::string	startLine("\n\r\n\n\r\nGET ///...////?q=key#fragment HTTP/1.1\r\n");
	std::string	request("\n\r\n\n\r\nGET /test/file.html HTTP/1.1\r\n"
							"Host: %4ehello%E2%82%AC:90\r\n"
							"User-Agent: Mozilla/5.0\n"
							"Accept: text/html, */*\r\n"
							"Accept-Language: en-us\n"
							"Accept-Charset: \tISO-8859-1,utf-8\n"
							"Connection: keep-alive , \r\n"
							"Keep-Alive: max=100,timeout=30\n"
							"Content-Length: 7 \n"
							"t: €h\n"
							"\r\n");
/* 	std::string	requestBody("7\r\n"
							"Mozilla\r\n"
							"11\r\n"
							"Developer Network\r\n"
							"0\r\n"
							"\r\n"); */
	std::string	requestBody("hello world");
	octets_t	octetRequest = convertStringToOctets(request);
	octets_t	octetRequestBody = convertStringToOctets(requestBody);
	HttpRequest	header;
	std::cout << "-----------------------REQUEST-----------------------" << std::endl;
	try
	{
		header.parseHeader(octetRequest);
		header.validateHeader();
		// if content-length, needs to wait for enough octets in buffer first, or time out
		header.readRequestBody(octetRequestBody);
	}
	catch(const std::invalid_argument& e)
	{
		std::cerr << e.what() << '\n';
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	
}