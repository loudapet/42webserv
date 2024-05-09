/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/07 10:46:07 by plouda            #+#    #+#             */
/*   Updated: 2024/05/09 16:04:01 by plouda           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <vector>
#include "../inc/HttpHeader.hpp"

octets_t	convertStringToOctets(std::string& str)
{
	octets_t vec(str.begin(), str.end());
	return (vec);
}

int	main()
{
	std::string	response = "HTTP/1.1  \t200     OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!\n";
	std::string	request = "GET /index.html HTTP/1.1\nHost: www.example.com\nUser-Agent: Mozilla/5.0\nAccept: text/html, */*\nAccept-Language: en-us\nAccept-Charset: ISO-8859-1,utf-8\nConnection: keep-alive\n";
	
	octets_t octetResponse = convertStringToOctets(response);
	octets_t octetRequest = convertStringToOctets(request);

	HttpHeader	header;
	std::cout << "-----------------------REQUEST-----------------------" << std::endl;
	header.parseHeader(octetRequest);
	std::cout << "----------------------RESPONSE-----------------------" << std::endl;
	header.parseHeader(octetResponse);
}