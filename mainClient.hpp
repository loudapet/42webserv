/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mainClient.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/27 16:29:49 by plouda            #+#    #+#             */
/*   Updated: 2024/06/27 16:33:57 by plouda           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MAINCLIENT_HPP
#define MAINCLIENT_HPP

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

static const char* request1 = "\n\r\n\nGET http://localhost/tours/index.html HTTP/1.0\r\n"
							"Host: s\r\n"
							"User-Agent: Mozilla/5.0\n"
							"Accept: text/html, */*\r\n"
							"Accept-Language: en-us\n"
							"Accept-Charset: \tISO-8859-1,utf-8\n"
							"Connection: keep-alive , \r\n"
							"Keep-Alive: max=100,timeout=30\n"
							"Expect: 100-continue\n\n"
							"GET /tours HTTP/1.0\r\n"
							"Host: localhost\r\n"
							"User-Agent: Mozilla/5.0\n"
							"Accept: text/html, */*\r\n"
							"Expect: 100-continue\n";
static const char* request2 = "Accept-Language: en-us\n"
							"Accept-Charset: \tISO-8859-1,utf-8\n"
							"Connection: keep-alive , \r\n"
							"Keep-Alive: max=100,timeout=30\n"
							"Transfer-Encoding: chunked\n"
							"t: €h\n"
							"\r\n"
							"8;\n"
							"Chromi\r\n\n"
							"12\r\n"
							"Developers Network\r\n"
							"0\r\n"
							"\r\n"
							"GET /../file_in_docs.html HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"User-Agent: Mozilla/5.0\n"
							"Accept: text/html, */*\r\n"
							"Accept-Language: en-us\n"
							"Accept-Charset: \tISO-8859-1,utf-8\n"
							"Connection: keep-alive , \r\n"
							"Keep-Alive: max=100,timeout=30\n"
							"Transfer-Encoding: chunked\n"
							"t: €h\n"
							"\r\n"
							"8;\n"
							"Chromium\r\n"
							"12\r\n"
							"Developers Network\r\n"
							"0\r\n"
							"\r\n"
							;
static const char* request3 = "GET /../file_in_docs.html HTTP/1.1\r\n"
							"Host: localhost\r\n"
							"User-Agent: Mozilla/5.0\n"
							"Accept: text/html, */*\r\n"
							"Accept-Language: en-us\n"
							"Accept-Charset: \tISO-8859-1,utf-8\n"
							"Connection: keep-alive , \r\n"
							"Keep-Alive: max=100,timeout=30\n"
							"Transfer-Encoding: chunked\n"
							"t: €h\n"
							"\r\n"
							"8;\n"
							"Chromium\r\n"
							"12\r\n"
							"Developers Network\r\n"
							"0\r\n"
							"\r\n"
							;


#endif  // MAINCLIENT_HPP
