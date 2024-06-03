/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   uri_validity_checker.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okraus <okraus@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/03 13:28:25 by okraus            #+#    #+#             */
/*   Updated: 2024/06/03 17:29:12 by okraus           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>
#include <sstream>
#include <cctype>

#define INVALID_URI 0
#define IP_V4 1
#define IP_V6 2
#define VALID_URI 4

//./a.out en.wikipedia-org.com 10.10.1.255 214.15.1.cn
//./a.out en.wikipedia-org-.com 10.10.1.256 214.15.1
//c++ -Wall -Wextra -Werror uri_validity_checker.cpp 


//what about octal, hexadecimal or integer IP???

bool	is_digits(const std::string str)
{
	if (str.empty())
		return (false);
	for (char ch : str)
		if (!std::isdigit(ch))
			return (false);
	return (true);
}

bool	is_alphas(const std::string str)
{
	if (str.empty())
		return (false);
	for (char ch : str)
		if (!std::isalpha(ch))
			return (false);
	return (true);
}


bool	is_alhypnumsperc(const std::string str)
{
	int	perc = 0;
	if (str.empty())
		return (false);
	for (char ch : str)
		if (!(std::isalnum(ch) || ch == '-'))
		{
			if (ch == '%' && !perc)
				perc = 2;
			else
				return (false);
		}
		else if (perc)
		{
			if (!std::isxdigit(ch))
				return (false);
			else
				--perc;
		}
	if (perc)
		return (false);
	return (true);
}

int	ok_strtoi(std::string str)
{
	std::stringstream	temp;
	int					num;

	temp << str;
	temp >> num;
	return (num);
}

bool	check_ipv4(std::string uri)
{
	int			octet;
	size_t		position;
	std::string	number;
	for (int i = 0; i < 3; ++i)
	{
		position = uri.find('.');
		if (position > 3)
			return (false);
		number = uri.substr(0, position);
		//std::cout << "position:" << position << std::endl;
		//std::cout << "number:" << number << std::endl;
		uri = uri.substr(position + 1);
		//std::cout << "uri:" << uri << std::endl;
		if (!is_digits(number))
			return (false);
		octet = ok_strtoi(number);
		if (octet > 255)
			return (false);
	}
	if (uri.length() > 3)
		return (false);
	if (!is_digits(uri))
		return (false);
	octet = ok_strtoi(uri);
	if (octet > 255)
		return (false);
	return (true);
}


//stuff to mostly ignore
//https://www.rfc-editor.org/rfc/rfc3492
//https://www.rfc-editor.org/rfc/rfc5891

// labels up to 63 characters
// labels Alnum - anh - alnum
// 3rd and 4th are not hyphen
//top level doamin has to be alpha only (or it needs to end with period)
bool	check_label(std::string label)
{
	//std::cout << "a\n";
	if (label.size() > 3 && label[2] == '-' && label[3] == '-')
		return (false);
	//std::cout << "aa\n";
	if (label[0] == '-' || *label.rbegin() == '-')
		return (false);
	//std::cout << label << " aaa\n";
	if (!is_alhypnumsperc(label))
		return (false);
	//std::cout << "true\n";
	return(true);
}

bool	check_valid(std::string uri)
{
	size_t		position;
	std::string	label;
	if (uri.size() > 255 || uri.size() == 0)
		return (false);
	while (uri.find('.') != std::string::npos)
	{
		position = uri.find('.');
		//std::cout << "position: " << position << std::endl;
		if (!position || uri[0] == '.')
			return (false);
		label = uri.substr(0, position);
		uri = uri.substr(position + 1);
		//std::cout << "label: " << label << std::endl;
		//std::cout << "uri: " << uri << std::endl;
		if (!check_label(label))
			return (false);
	}
	if (uri.size() && !is_alphas(uri))
		return (false);
	return (true);
}

int	identify_uri(std::string uri)
{
	if (check_ipv4(uri))
	{
		std::cout << "String <" << uri << "> was successfully checked as IPv4" << std::endl;
		return (IP_V4);
	}
	if (check_valid(uri))
	{
		std::cout << "String <" << uri << "> was successfully checked as valid" << std::endl;
		return (VALID_URI);
	}
	return (INVALID_URI);
}

int	main(int argc, char *argv[])
{
	int	type;
	if (argc == 1)
	{
		std::cout << "Not enugh arguments" << std::endl;
		return (1);
	}
	for (int i = 1; argv[i]; ++i)
	{
		std::cout << std::endl << argv[i] << std::endl;
		type = identify_uri(argv[i]);
		if (type == INVALID_URI)
			std::cout << "INVALID_URI" << std::endl;
		else if (type == IP_V4)
			std::cout << "IP_V4" << std::endl;
		else if (type == IP_V6)
			std::cout << "IP_V6" << std::endl;
		else if (type == VALID_URI)
			std::cout << "VALID_URI" << std::endl;
		else
			std::cout << "This is impossible!" << std::endl;
		std::cout << std::endl;
	}
	return (0);
}