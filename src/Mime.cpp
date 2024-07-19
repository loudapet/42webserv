/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Mime.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/18 13:57:01 by plouda            #+#    #+#             */
/*   Updated: 2024/07/19 13:17:53aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Mime.hpp"

Mime::Mime()
{
	this->_mimeTypesDict = std::map< std::string, std::set<std::string> >();
	return ;
}

Mime::Mime(const Mime& refObj)
{
	*this = refObj;
}

Mime& Mime::operator = (const Mime& refObj)
{
	if (this != &refObj)
		this->_mimeTypesDict = refObj._mimeTypesDict;
	return (*this);
}

Mime::~Mime()
{
	return ;
}

const std::map< std::string, std::set<std::string> > &Mime::getMimeTypesDict() const
{
	return (this->_mimeTypesDict);
}



bool checkSemicolons(const std::string& str) {
	size_t start = 0; // Start of the current chunk
	size_t end; // End of the current chunk

	while (start < str.size())
	{
		// Find the end of the current chunk
		end = str.find('\n', start);
		if (end == std::string::npos) {
			end = str.size(); // If no newline is found, the end is the end of the string
		}

		// Check if the chunk is empty or contains only whitespaces
		if (str.find_first_not_of(WHITESPACES, start) >= end) {
			// Move to the next chunk if the current chunk is empty or contains only whitespaces
			start = end + 1;
			continue;
		}

		// Find the last non-whitespace character in the current chunk
		size_t lastNonWhitespace = str.find_last_not_of(WHITESPACES, end - 1);
		if (lastNonWhitespace == std::string::npos || str[lastNonWhitespace] != ';') {
			// If no non-whitespace character is found or it's not a semicolon, return false
			return false;
		}

		// Move to the next chunk
		start = end + 1;
	}

	// If all non-empty chunks end with a semicolon (ignoring whitespaces), return true
	return true;
}



void Mime::parseMimeTypes(const std::string &mimeTypesFilePath)
{
	std::ifstream				file;
	char						c;
	std::stringstream			tmpFileContent;
	std::string					mimeTypesFileContent;
	std::vector<std::string>	mimeTypes;
	std::istringstream			iss;
	std::string					element;
	std::string					key;
	std::set<std::string>	extensions;

	if (access(mimeTypesFilePath.c_str(), R_OK) < 0)
			return ;
	file.open(mimeTypesFilePath.c_str());
	if (!(file >> c)) // check if the file is empty by trying to read a character from it
		return ;
	file.putback(c); //	putting the character back bcs it will be read again later
	tmpFileContent << file.rdbuf();
	file.close();
	mimeTypesFileContent = tmpFileContent.str();
	if (!checkSemicolons(mimeTypesFileContent))
	{
		std::cerr << "Warning: Config Parser: Mime types file is not properly formatted and won't be loaded into the server." << std::endl;
		return ;
	}
	iss.str(mimeTypesFileContent);
	while (iss >> element)
		mimeTypes.push_back(element);
	for (size_t i = 0; i < mimeTypes.size(); i++)
	{
		if (key.empty())
			key = mimeTypes[i];
		else
		{
			if (mimeTypes[i][mimeTypes[i].size() - 1] == ';')
			{
				mimeTypes[i] = mimeTypes[i].substr(0, mimeTypes[i].size() - 1); // remove semicolon
				extensions.insert(mimeTypes[i]);
				if (this->_mimeTypesDict.find(key) == this->_mimeTypesDict.end())
					this->_mimeTypesDict[key] = extensions;
				else
					this->_mimeTypesDict[key].insert(extensions.begin(), extensions.end());
				key.clear();
				extensions.clear();
			}
			else
				extensions.insert(mimeTypes[i]);
		}
	}
}
