/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouda <plouda@student.42prague.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 10:52:29 by plouda            #+#    #+#             */
/*   Updated: 2024/06/21 16:36:36 by plouda           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/HttpResponse.hpp"
#include "../inc/HttpRequest.hpp"

HttpResponse::HttpResponse()
{
	this->statusLine.httpVersion = "HTTP/1.1";
	this->statusLine.statusCode = 200;
	this->statusLine.reasonPhrase = "OK";
	this->headerFields["Server: "] = "webserv/nginx-but-better";
	this->codeDict[100] = "Continue";
	this->codeDict[200] = "OK";
	this->codeDict[201] = "Created";
	this->codeDict[202] = "Accepted";
	this->codeDict[301] = "Moved Permanently";
	this->codeDict[302] = "Found";
	this->codeDict[303] = "See Other";
	this->codeDict[307] = "Temporary Redirect";
	this->codeDict[308] = "Permanent Redirect";
	this->codeDict[400] = "Bad Request";
	this->codeDict[403] = "Forbidden";
	this->codeDict[404] = "Not Found";
	this->codeDict[405] = "Method Not Allowed";
	this->codeDict[408] = "Request Timeout";
	this->codeDict[411] = "Length Required";
	this->codeDict[413] = "Content Too Large";
	this->codeDict[414] = "URI Too Long";
	this->codeDict[415] = "Unsupported Media Type";
	this->codeDict[417] = "Expectation Failed";
	this->codeDict[421] = "Misdirected Request";
	this->codeDict[500] = "Internal Server Error";
	this->codeDict[501] = "Not Implemented";
	this->codeDict[505] = "HTTP Version Not Supported";
	this->responseBody = octets_t();
	this->completeResponse = octets_t();
	this->statusLocked = false;
	return ;
}

// Sun, 06 Nov 1994 08:49:37 GMT 
std::string	getIMFFixdate(void)
{
	time_t		curr_time;
	tm			*curr_tm;
	char		buffer[100];

	std::time(&curr_time);
	curr_tm = std::gmtime(&curr_time);
	std::strftime(buffer, 100, "%a, %d %b %Y %H:%M:%S GMT", curr_tm);
	return (std::string(buffer));
}

HttpResponse::HttpResponse(const HttpResponse& refObj)
{
	*this = refObj;
}

HttpResponse& HttpResponse::operator=(const HttpResponse& refObj)
{
	if (this != &refObj)
	{
		statusLine = refObj.statusLine;
		statusDetails = refObj.statusDetails;
		headerFields = refObj.headerFields;
		responseBody = refObj.responseBody;
		completeResponse = refObj.completeResponse;
		codeDict = refObj.codeDict;
	}
	return (*this);
}

HttpResponse::~HttpResponse()
{
	return ;
}

const statusLine_t	&HttpResponse::getStatusLine() const
{
	return (this->statusLine);
}

const unsigned short&	HttpResponse::getStatusCode() const
{
	return (this->statusLine.statusCode);
}

const bool &HttpResponse::getStatusLocked() const
{
	return (this->statusLocked);
}

// overrides updateStatus()
void	HttpResponse::setStatusCode(unsigned short code)
{
	this->statusLine.statusCode = code;
}

void HttpResponse::updateStatus(unsigned short code, const char* details)
{
	if (!this->statusLocked)
	{
		this->statusLine.statusCode = code;
		this->statusDetails = details;
	}
	this->statusLocked = true;
}

void HttpResponse::setStatusLineAndDetails(const statusLine_t &incStatusLine, const std::string &details)
{
	this->statusLine.httpVersion = incStatusLine.httpVersion;
	this->statusLine.statusCode = incStatusLine.statusCode;
	this->statusLine.reasonPhrase = this->codeDict[incStatusLine.statusCode];
	this->statusDetails = details;
}

void	HttpResponse::lockStatusCode()
{
	this->statusLocked = true;
}

// needs different Content-Type
// needs proper allowed methods handling
void	HttpResponse::buildResponseHeaders(const HttpRequest& request)
{
	size_t		contentLength = this->responseBody.size();
	std::string	date = getIMFFixdate();
	std::string	type("text/html");
	this->headerFields["Content-Length: "] = itoa(contentLength);
	this->headerFields["Date: "] = date;
	this->headerFields["Content-Type: "] = type;
	if (request.getConnectionStatus() == CLOSE)
		this->headerFields["Connection: "] = "close";
	else
	{
		this->headerFields["Connection: "] = "keep-alive";
		this->headerFields["Keep-Alive: "] = std::string("timeout=" + itoa(CONNECTION_TIMEOUT));
	}
	if (this->statusLine.statusCode >= 300 && this->statusLine.statusCode <= 308)
	{
		this->headerFields["Location: "] = request.getLocation().getReturnURLOrBody();
	}
	if (this->statusLine.statusCode == 405)
	{
		std::string	methods;
		std::set<std::string>::const_iterator it = request.getAllowedMethods().begin();
		while (it != request.getAllowedMethods().end())
		{
			methods.append(*it);
			if (++it != request.getAllowedMethods().end())
				methods.append(", ");
		}
		this->headerFields["Allow: "] = methods;
	}
	for (stringmap_t::iterator it = this->headerFields.begin() ; it != this->headerFields.end() ; it++)
		it->second.append(CRLF);
}

void	HttpResponse::readErrorPage(const Location &location)
{
	const std::map<unsigned short, std::string> &errorPages = location.getErrorPages();
	std::ifstream											errorPageFile;
	std::stringstream										buff;
	std::map<unsigned short, std::string>::const_iterator	it;
	std::stringstream										ss;
	
	it = errorPages.find(this->statusLine.statusCode);
	if (it != errorPages.end())
	{
		errorPageFile.open(it->second.c_str());
		if (errorPageFile)
		{
			buff << errorPageFile.rdbuf();
			this->responseBody = convertStringToOctets(buff.str());
			//std::cout << CLR2 << "Response body: " << this->responseBody << RESET << std::endl;
		}
	}
	ss << "<html>\r\n"
	   << "<head><title>" << this->statusLine.statusCode << " " << this->statusLine.reasonPhrase << "</title></head>\r\n"
	   << "<body>\r\n"
	   << "<center><h1>" << this->statusLine.statusCode << " " << this->statusLine.reasonPhrase << "</h1></center>\r\n"
	   << "<center><h2>" << this->statusDetails << "</h2></center>\r\n"
	   << "</html>\r\n";
	this->responseBody = convertStringToOctets(ss.str());
	//std::cout << CLR2 << "Response body: " << this->responseBody << RESET << std::endl;
}

void HttpResponse::readRequestedFile(const std::string &targetResource)
{
	std::ifstream	stream(targetResource.c_str(), std::ios::binary);
	octets_t		contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
	this->responseBody.insert(this->responseBody.end(), contents.begin(), contents.end());
}

void	HttpResponse::buildCompleteResponse(void)
{
	octets_t	httpVersion(convertStringToOctets(this->statusLine.httpVersion));
	octets_t	statusCode(convertStringToOctets(itoa(this->statusLine.statusCode)));
	octets_t	reasonPhrase(convertStringToOctets(this->statusLine.reasonPhrase));
	octets_t	delimiter(convertStringToOctets(CRLF));
	this->completeResponse.insert(this->completeResponse.end(), httpVersion.begin(), httpVersion.end());
	this->completeResponse.push_back(' ');
	this->completeResponse.insert(this->completeResponse.end(), statusCode.begin(), statusCode.end());
	this->completeResponse.push_back(' ');
	this->completeResponse.insert(this->completeResponse.end(), reasonPhrase.begin(), reasonPhrase.end());
	this->completeResponse.insert(this->completeResponse.end(), delimiter.begin(), delimiter.end());

	for (stringmap_t::iterator it = this->headerFields.begin() ; it != this->headerFields.end() ; it++)
	{
		this->completeResponse.insert(this->completeResponse.end(), it->first.begin(), it->first.end());
		this->completeResponse.insert(this->completeResponse.end(), it->second.begin(), it->second.end());
	}
	this->completeResponse.insert(this->completeResponse.end(), delimiter.begin(), delimiter.end()); // insert extra CRLF

	if (this->responseBody.size() > 0)
		this->completeResponse.insert(this->completeResponse.end(), this->responseBody.begin(), this->responseBody.end());
}

void	HttpResponse::readDirectoryListing(const std::string& targetResource)
{
	std::stringstream	body;
	DIR*				dirPtr;
	dirPtr = opendir(targetResource.c_str());
	body << "<html>\r\n"
	   << "<head><title>" << "Index of " << targetResource << "</title></head>\r\n"
	   << "<body>\r\n"
	   << "<h1>" << "Index of " << targetResource << "</h1>\r\n"
	   << "<hr><pre>\r\n";
	for (dirent* dir = readdir(dirPtr); dir != NULL; dir = readdir(dirPtr))
	{
		std::string	path = targetResource + dir->d_name;
		struct stat	fileCheckBuff;
		if (stat(path.c_str(), &fileCheckBuff) < 0)
			std::cout << errno << " "<< path << std::endl;
		tm *curr_tm = std::gmtime(&(fileCheckBuff.st_mtim.tv_sec));
		char time[100];
		std::strftime(time, 100, "%a, %d %b %Y %H:%M:%S GMT", curr_tm);

		if (dir->d_type == DT_DIR)
		{
			body << "<a href=\"" << dir->d_name << "/\">" << dir->d_name << "/" << "</a>";
			body << " " << time << " " << "-" << "\r\n";
		}
		else if (dir->d_type == DT_REG)
		{
			body << "<a href=\"" << dir->d_name << "\">" << dir->d_name << "" << "</a>";
			body << " " << time << " " << fileCheckBuff.st_size << "\r\n";
		}
	}
	body << "</pre><hr></body>\r\n";
	body << "</html>\r\n";
	this->responseBody = convertStringToOctets(body.str());
	closedir(dirPtr);
}
void	HttpResponse::readReturnDirective(const Location &location)
{
	this->responseBody = convertStringToOctets(location.getReturnURLOrBody());
}


const octets_t		HttpResponse::prepareResponse(HttpRequest& request)
{
	if (codeDict.find(this->statusLine.statusCode) == codeDict.end())
		this->codeDict[this->statusLine.statusCode] = "Undefined";
	this->statusLine.reasonPhrase = this->codeDict[this->statusLine.statusCode];
	if (this->statusLine.statusCode == 200 && request.getTargetIsDirectory())
		request.response.readDirectoryListing(request.getTargetResource());
	else if (this->statusLine.statusCode == 200)
		request.response.readRequestedFile(request.getTargetResource());
	else if (request.getLocation().getIsRedirect() && (this->statusLine.statusCode < 300 || this->statusLine.statusCode > 308))
		request.response.readReturnDirective(request.getLocation());
	else
		request.response.readErrorPage(request.getLocation());
	request.response.buildResponseHeaders(request);
	request.response.buildCompleteResponse();
	octets_t message = request.response.getCompleteResponse();
	return (message);
}

const octets_t &HttpResponse::getCompleteResponse() const
{
	return (this->completeResponse);
}
