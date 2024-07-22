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
	this->_mimeTypesDictInv = stringmap_t();
}

Mime::Mime(const Mime& refObj)
{
	*this = refObj;
}

Mime& Mime::operator = (const Mime& refObj)
{
	if (this != &refObj)
		this->_mimeTypesDict = refObj._mimeTypesDict;
		this->_mimeTypesDictInv = refObj._mimeTypesDictInv;
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

const stringmap_t &Mime::getMimeTypesDictInv() const
{
	return (this->_mimeTypesDictInv);
}


bool checkSemicolons(const std::string& str)
{
	size_t	start;
	size_t	end;

	start = 0;
	while (start < str.size())
	{
		end = str.find('\n', start);
		if (end == std::string::npos)
			end = str.size();
		if (str.find_first_not_of(WHITESPACES, start) >= end)
		{
			start = end + 1;
			continue;
		}
		size_t lastNonWhitespace = str.find_last_not_of(WHITESPACES, end - 1);
		if (lastNonWhitespace == std::string::npos || str[lastNonWhitespace] != ';')
			return (false);
		start = end + 1;
	}
	return (true);
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
		Logger::log(WARNING, CONFIG, "Config Parser: Mime types file is not properly formatted and won't be loaded into the server.", "");
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
	if (this->_mimeTypesDict.size() > 0)
		for (std::map< std::string, std::set<std::string> >::iterator it = this->_mimeTypesDict.begin(); it != this->_mimeTypesDict.end(); it++)
			if (it->second.size() > 0)
				for (std::set<std::string>::iterator itr = it->second.begin(); itr != it->second.end(); itr++)
					this->_mimeTypesDictInv[*itr] = it->first;

	// std::map<std::string, std::set<std::string> >::const_iterator mapIt;
    // for (mapIt = this->_mimeTypesDict.begin(); mapIt != this->_mimeTypesDict.end(); ++mapIt)
    // {
    //     std::cout << mapIt->first << ": ";
    //     std::set<std::string>::const_iterator setIt;
    //     for (setIt = mapIt->second.begin(); setIt != mapIt->second.end(); ++setIt)
    //         std::cout << *setIt << " ";
    //     std::cout << "|" << std::endl;
    // }
}
