/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/02 18:05:06 by aulicna           #+#    #+#             */
/*   Updated: 2024/09/02 15:04:19 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/webserv.hpp"
#include "../inc/ResponseException.hpp"

bool	validateElement(std::string &element)
{
	size_t	pos;

	pos = element.rfind(";");
	if (pos != element.size() - 1)
		throw(std::runtime_error("Config parser: Invalid ending of an element in a server block."));
	element.erase(pos);
	return (true);
}

unsigned short	validateListen(unsigned short port, const std::string &portFromConfig)
{
	unsigned short		newPort;
	std::istringstream	iss; // convert listen port to unsigned short

	if (port != 0)
		throw (std::runtime_error("Config parser: Duplicate listen directive."));
	for (int j = 0; portFromConfig[j]; j++)
	{
		if (!std::isdigit(portFromConfig[j]))
			throw (std::runtime_error("Config parser: Invalid port."));
	}
	iss.str(portFromConfig);
	if (!(iss >> newPort) || !iss.eof())
		throw(std::runtime_error("Config parser: Port number is out of range for unsigned short."));
	iss.str("");
	iss.clear();
	return (newPort);
}

std::string		validateRoot(const std::string &root, const std::string &rootFromConfig, const std::string &exceptionScope)
{
	if (!root.empty())
		throw (std::runtime_error("Config parser: Duplicate root directive in a " + exceptionScope + " scope."));
	if (exceptionScope == "server")
		return (dirIsValidAndAccessible(rootFromConfig, 
			"Cannot access root path of a server.", "Root path of a server is not a directory."));
	else
		return (dirIsValidAndAccessible(rootFromConfig, 
			"Cannot access root path of a location.", "Root path of a location is not a directory."));
}

int	validateRequestBodySizeLimit(bool rbslInConfig, const std::string &rbslFromConfig, const std::string &exceptionMessage)
{
	int		multiplier;
	long	rbslValue;
	char	*end;

	multiplier = 1;
	if (rbslInConfig)
		throw (std::runtime_error("Config parser: Duplicate client_max_body_size directive in a " + exceptionMessage + " scope."));
	for (size_t j = 0; j < rbslFromConfig.size(); j++)
	{
		if (!std::isdigit(rbslFromConfig[j]))
			throw (std::runtime_error("Config parser: Invalid client_max_body_size number in a " + exceptionMessage + " scope."));
	}
	rbslValue = strtol(rbslFromConfig.c_str(), &end, 10);
	if (end[0] != 'K' && end[0] != 'M' && end[0] != '\0')
		throw (std::runtime_error("Config parser: Invalid client_max_body_size number."));
	if (end[0] == 'K')
	{
		if (rbslValue > INT_MAX / 1024 || rbslValue < 0)
			throw(std::overflow_error("Config parser: client_max_body_size too large."));
		multiplier = 1024;
	}
	else if (end[0] == 'M')
	{
		if (rbslValue > INT_MAX / (1024 * 1024) || rbslValue < 0)
			throw(std::overflow_error("Config parser: client_max_body_size too large."));
		multiplier = 1024 * 1024;
	}
	else if (end[0] == '\0' && rbslValue > INT_MAX)
		throw(std::overflow_error("Config parser: client_max_body_size too large."));
	return (static_cast<int>(rbslValue) * multiplier);
}

bool	validateOnOffDirective(bool isDirectiveInConfig, const std::string &directiveFromConfig, const std::string &directiveValueFromConfig, const std::string &exceptionMessage)
{
	if (isDirectiveInConfig)
		throw (std::runtime_error("Config parser: Duplicate " + directiveFromConfig + " directive in a " + exceptionMessage + " scope."));
	if (directiveValueFromConfig != "on" && directiveValueFromConfig != "off")
		throw (std::runtime_error("Config parser: Wrong syntax of " + directiveFromConfig + " directive in a " + exceptionMessage + " scope."));
	if (directiveValueFromConfig == "on")
		return (true);
	return (false);
}

std::vector<std::string>	validateIndex(const std::vector<std::string> &index, const std::vector<std::string> &scopeElements, size_t pos, const std::string &exceptionMessage)
{
	std::vector<std::string>	newIndex;

	if (!(index.size() == 0))
		throw (std::runtime_error("Config parser: Duplicate index directive in a " + exceptionMessage + " scope."));
	newIndex = extractVectorUntilSemicolon(scopeElements, pos);
	validateElement(newIndex.back());
	return (newIndex);
}

unsigned short	validateReturnCode(std::string &scopeElement)
{
	std::istringstream	iss;
	unsigned short		returnCode;

	for (size_t i = 0; i < scopeElement.size(); i++)
	{
		if (!std::isdigit(scopeElement[i]))
			throw (std::runtime_error("Config parser: Invalid return code."));
	}
	iss.str(scopeElement);
	if (!(iss >> returnCode) || !iss.eof())
		throw(std::runtime_error("Config parser: Return code is out of range for valid return codes."));
	iss.str("");
	iss.clear();
	if (returnCode < 100 || returnCode > 599)
		throw(std::runtime_error("Config parser: Return code is out of range of valid return codes."));
	return (returnCode);
}

std::vector<std::string>	extractVectorUntilSemicolon(const std::vector<std::string> &mainVector, size_t pos)
{
	std::vector<std::string>	extractedVector;

	size_t j = 0;
	while (pos + j < mainVector.size() && mainVector[pos + j].find(';') == std::string::npos)
	{
		extractedVector.push_back(mainVector[pos + j]);
		j++;
	}
	if (pos + j < mainVector.size())
		extractedVector.push_back(mainVector[pos + j]);
	return (extractedVector);
}

void	fileIsValidAndAccessible(const std::string &path, const std::string &fileName)
{
	if (access(path.c_str(), F_OK) < 0)
		throw(std::runtime_error("Config parser: " + fileName + " file at '" + path + "' is an invalid file."));
	if (access(path.c_str(), R_OK) < 0)
		throw(std::runtime_error("Config parser: " + fileName + " file at '" + path + "' is not accessible."));
}

std::string	dirIsValidAndAccessible(const std::string &path, const std::string &accessMessage, const std::string &dirOrFileMessage)
{
	int			fileCheck;
	struct stat	fileCheckBuff;

	fileCheck = stat(path.c_str(), &fileCheckBuff);
	if (fileCheck != 0)
		throw(std::runtime_error("Config parser: " + accessMessage));
	if (!(fileCheckBuff.st_mode & S_IFDIR) || fileCheckBuff.st_mode & S_IFREG) // not a directory, but a file
		throw(std::runtime_error("Config parser: " + dirOrFileMessage));
	if (path[path.size() - 1] != '/')
		return (path + "/");
	return (path);
}

std::string	resolveDotSegments(std::string path, ServerSection flag)
{
	std::stack<std::string>	segments;
	std::stack<std::string>	output;
	std::stringstream	input(path);
	std::string			buffer;
	std::string			newPath;
	if (flag == CONFIG)
		output.push("./");
	else
		output.push("/");
	while (std::getline(input, buffer, '/'))
	{
		if (buffer == "..")
		{
			if (output.size() > 1)
				output.pop();
			else if (flag == CONFIG)
				throw(std::invalid_argument("Config parser: Path above root.")); // will be caught in try catch in main
			else
				throw(ResponseException(400, "Invalid relative path segment"));
		}
		else if (buffer != "." && buffer != "")
			output.push(buffer + std::string("/"));
	}
	if (*(output.top().rbegin()) == '/' && *(path.rbegin()) != '/' && output.top().size() > 0)
		output.top().erase(output.top().size() - 1, 1);
	while (output.size() > 0)
	{
		newPath.insert(0, output.top());
		output.pop();
	}
	return (newPath);
}

bool hasValidHeaderEnd(const octets_t &receivedData)
{
	std::string sequences[] = {"\r\n\n", "\n\n", "\n\r\n", "\r\n\r\n"};
	std::vector<unsigned char>::const_iterator endOfSequence;

	for (int i = 0; i < 4; ++i)
	{
		endOfSequence = std::search(receivedData.begin(), receivedData.end(), sequences[i].begin(), sequences[i].end());
		if (endOfSequence != receivedData.end())
			return (true);
	}
	return (false);
}

octets_t	convertStringToOctets(std::string str)
{
	octets_t vec(str.begin(), str.end());
	return (vec);
}

std::string convertOctetsToString(const octets_t& octets)
{
    return (std::string(octets.begin(), octets.end()));
}

void	logSockets(int socket, std::string action)
{
	std::ofstream	file("sockets_log.txt", std::ios::app);
	if (!file)
		std::cerr << "Error opening file." << std::endl;
	else
		file  << action << " socket " << socket << std::endl;
}

std::string	trim(const std::string& str)
{
	const std::string	whitespace(" \t\r\n");
	const size_t		strBegin = str.find_first_not_of(whitespace);
	if (strBegin == std::string::npos)
		return (""); // no content
	const size_t strEnd = str.find_last_not_of(whitespace);
	const size_t strRange = strEnd - strBegin + 1;
	return (str.substr(strBegin, strRange));
}

std::vector<std::string>	splitQuotedString(const std::string& str, char sep)
{
	if (std::count(str.begin(), str.end(), '\"') % 2)
		throw(ResponseException(400, "Unclosed quotes in quoted string"));
	std::vector<std::string> splitString;
	unsigned int counter = 0;
	std::string segment;
	std::stringstream stream_input(str);
	while(std::getline(stream_input, segment, '\"'))
	{
		++counter;
		if (counter % 2 == 0)
		{
			if (!segment.empty())
				splitString.push_back(segment);
		}
		else
		{
			std::stringstream stream_segment(segment);
			while(std::getline(stream_segment, segment, sep))
				if (!segment.empty())
					splitString.push_back(segment);
		}
	}
	return (splitString);
}

std::vector<std::string>	splitBlock(std::string &block)
{
	std::vector<std::string>	blockElements;
	size_t						start;
	size_t						end;

	start = 0;
	end = block.find_first_not_of(WHITESPACES);
	while (end != std::string::npos)
	{
		start = block.find_first_not_of(WHITESPACES, start);
		end = block.find_first_of(WHITESPACES, start);
		if (start != std::string::npos)
		{
			if (end != std::string::npos)
				blockElements.push_back(block.substr(start, end - start));
			else
				blockElements.push_back(block.substr(start, block.length() - start));
		}
		start = end + 1;
	}
	return (blockElements);
}

void	removeDoubleSlash(std::string& str)
{
	size_t pos;
	while ((pos = str.find("//")) != std::string::npos)
		str.erase(pos, 1);
}