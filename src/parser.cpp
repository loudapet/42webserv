/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/07 10:46:07 by plouda            #+#    #+#             */
/*   Updated: 2024/05/10 15:37:02 by plouda           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <vector>
#include <string>
#include "../inc/HttpHeader.hpp"

octets_t	convertStringToOctets(std::string& str)
{
	octets_t vec(str.begin(), str.end());
	return (vec);
}

std::string getFileContents(const char *filename)
{
  std::ifstream in(filename, std::ios::in | std::ios::binary);
  if (in)
  {
    std::string contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], contents.size());
    in.close();
    return(contents);
  }
  return (NULL);
}

int	main()
{
	//std::string	request = getFileContents(argv[1]);
	//std::cout << request << std::endl;
	std::string	startLine = 		"\n\r\n\n\r\nGET /index.html HTTP/1.1\r\n";
	std::string	host = 				"Host: www.example.com\r\n";
	std::string userAgent = 		"User-Agent: Mozilla/5.0\n";
	std::string	accept = 			"Accept: text/html, */*\r\n";
	std::string	acceptLanguage = 	"Accept-Language: en-us\n";
	std::string	acceptCharset = 	"Accept-Charset: ISO-8859-1,utf-8\n";
	std::string	connection = 		"Connection: keep-alive\r\n";
	std::string	endLine = 			"\n";
	std::string	request = startLine + host + userAgent + accept + acceptLanguage + acceptCharset + connection + endLine;
	//request = "\n";

	std::string		response = "HTTP/1.1  \t200     OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!\n";
	octets_t octetRequest = convertStringToOctets(request);
	octets_t octetResponse = convertStringToOctets(response);

	HttpHeader	header;
	std::cout << "-----------------------REQUEST-----------------------" << std::endl;
	header.parseHeader(octetRequest);
	std::cout << "----------------------RESPONSE-----------------------" << std::endl;
	header.parseHeader(octetResponse);
}