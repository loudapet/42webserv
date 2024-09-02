/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aulicna <aulicna@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 10:52:29 by plouda            #+#    #+#             */
/*   Updated: 2024/09/02 10:23:04 by aulicna          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/HttpResponse.hpp"
#include "../inc/HttpRequest.hpp"

HttpResponse::HttpResponse()
{
	this->statusLine.httpVersion = "HTTP/1.1";
	this->statusLine.statusCode = 200;
	this->statusLine.reasonPhrase = "OK";
	this->headerFields["server: "] = "webserv/nginx-but-better";
	this->codeDict[100] = "Continue";
	this->codeDict[200] = "OK";
	this->codeDict[201] = "Created";
	this->codeDict[202] = "Accepted";
	this->codeDict[203] = "Non-Authoritative Information";
	this->codeDict[204] = "No Content";
	this->codeDict[205] = "Reset Content";
	this->codeDict[206] = "Partial Content";
	this->codeDict[300] = "Multiple Choices";
	this->codeDict[301] = "Moved Permanently";
	this->codeDict[302] = "Found";
	this->codeDict[303] = "See Other";
	this->codeDict[304] = "Not Modified";
	this->codeDict[305] = "Use Proxy";
	this->codeDict[306] = "Unused";
	this->codeDict[307] = "Temporary Redirect";
	this->codeDict[308] = "Permanent Redirect";
	this->codeDict[400] = "Bad Request";
	this->codeDict[401] = "Unauthorized";
	this->codeDict[402] = "Payment required";
	this->codeDict[403] = "Forbidden";
	this->codeDict[404] = "Not Found";
	this->codeDict[405] = "Method Not Allowed";
	this->codeDict[406] = "Not Acceptable";
	this->codeDict[407] = "Proxy Authentication Required";
	this->codeDict[408] = "Request Timeout";
	this->codeDict[409] = "Conflict";
	this->codeDict[410] = "Gone";
	this->codeDict[411] = "Length Required";
	this->codeDict[412] = "Precondition Failed";
	this->codeDict[413] = "Content Too Large";
	this->codeDict[414] = "URI Too Long";
	this->codeDict[415] = "Unsupported Media Type";
	this->codeDict[416] = "Range Not Satisfiable";
	this->codeDict[417] = "Expectation Failed";
	this->codeDict[418] = "Unused";
	this->codeDict[421] = "Misdirected Request";
	this->codeDict[422] = "Unprocessable Content";
	this->codeDict[426] = "Upgrade Required";
	this->codeDict[500] = "Internal Server Error";
	this->codeDict[501] = "Not Implemented";
	this->codeDict[502] = "Bad Gateway";
	this->codeDict[503] = "Service Unavailable";
	this->codeDict[504] = "Gateway Timeout";
	this->codeDict[505] = "HTTP Version Not Supported";
	this->responseBody = octets_t();
	this->completeResponse = octets_t();
	this->statusLocked = false;
	this->message = octets_t();
	this->messageTooLongForOneSend = false;
	this->cgiStatus = 0;
	this->postStatus = 0;
	this->cgi_pid = 0;
	this->wfd = 0;
	this->rfd = 0;
	this->cgiHeaderFields = stringmap_t();
	this->cgiBody = octets_t();
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
		statusLocked = refObj.statusLocked;
		message = refObj.message;
		messageTooLongForOneSend = refObj.messageTooLongForOneSend;
		cgiStatus = refObj.cgiStatus;
		postStatus = refObj.postStatus;
		cgi_pid = refObj.cgi_pid;
		wfd = refObj.wfd;
		rfd = refObj.rfd;
		cgiHeaderFields = refObj.cgiHeaderFields;
		cgiBody = refObj.cgiBody;
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
		//Logger::safeLog(INFO, REQUEST, itoa(this->statusLine.statusCode) + " ", this->statusDetails);
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

void	HttpResponse::buildResponseHeaders(const HttpRequest& request)
{
	if (this->cgiStatus)
	{
		stringmap_t::iterator it = this->cgiHeaderFields.begin();
		for ( ; it != this->cgiHeaderFields.end() && it->first != "status: "; it++)
			if (!(this->headerFields.insert(std::make_pair(it->first, it->second)).second))  // overwrite with CGI values if exists
				this->headerFields[it->first] = it->second;
	}
	if (request.getRequestLine().httpVersion == "1.0")
	{
		this->statusLine.httpVersion = "HTTP/1.0";
		if (this->headerFields.find("transfer-encoding: ") != this->headerFields.end()) // this will ensure compliance with 1.0, but will likely invalidate the response client-side
			this->headerFields.erase(this->headerFields.find("transfer-encoding: "));
	}
	if (this->statusLine.statusCode != 204 && this->statusLine.statusCode != 304 && request.getRequestLine().method != "HEAD")
		this->headerFields.insert(std::make_pair("content-length: ", itoa(this->responseBody.size())));
	this->headerFields.insert(std::make_pair("date: ", getIMFFixdate()));
	if (this->headerFields.find("content-type: ") == this->headerFields.end() 
		&& this->statusLine.statusCode != 204 && this->statusLine.statusCode != 304
		&& request.getRequestLine().method != "HEAD")
		this->headerFields.insert(std::make_pair("content-type: ", "application/octet-stream"));
	if (request.getConnectionStatus() == CLOSE)
		this->headerFields.insert(std::make_pair("connection: ", "close"));
	else
	{
		this->headerFields.insert(std::make_pair("connection: ", "keep-alive"));
		this->headerFields.insert(std::make_pair("keep-alive: ", std::string("timeout=" + itoa(CONNECTION_TIMEOUT))));
	}
	if (this->statusLine.statusCode >= 300 && this->statusLine.statusCode <= 308)
	{
		if (this->statusDetails == "Trying to access a directory")
			this->headerFields.insert(std::make_pair("location: ", request.getAbsolutePath() + "/"));
		else if (request.getLocation().getReturnURLOrBody().size() > 0 
			&& *request.getLocation().getReturnURLOrBody().begin() == '/')	// if starts with slash, prepend scheme + host:port
			this->headerFields.insert(std::make_pair("location: ", std::string("http://") + request.getLocation().getServerName() + ":" + itoa(request.getLocation().getPort()) + request.getLocation().getReturnURLOrBody()));
		else
			this->headerFields.insert(std::make_pair("location: ", request.getLocation().getReturnURLOrBody()));
		// if starts with http(s), append the rest without http(s)
		// if starts with http(s)://host:port, append path
	}
	else if (this->statusLine.statusCode == 201)
		this->headerFields.insert(std::make_pair("location: ", request.getRequestLine().requestTarget.absolutePath));
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
		this->headerFields.insert(std::make_pair("allow: ", methods));
		//this->headerFields["allow: "] = methods;
	}
	if (this->statusLine.statusCode == 426)
	{
		this->headerFields["connection: "].append(", Upgrade");
		this->headerFields.insert(std::make_pair("upgrade: ", "HTTP/1.1"));
		//this->headerFields["upgrade: "] = "HTTP/1.1";
	}
	for (stringmap_t::iterator it = this->headerFields.begin() ; it != this->headerFields.end() ; it++)
		it->second.append(CRLF);
	// if (this->cgiStatus)
	// 	for (stringmap_t::iterator it = this->cgiHeaderFields.begin(); it != this->cgiHeaderFields.end(); it++)
	// 		if (!(this->headerFields.insert(std::make_pair(it->first, it->second)).second))  // overwrite with CGI values if exists
	// 			this->headerFields[it->first] = it->second;
}

void	HttpResponse::readErrorPage(const Location &location)
{
	const std::map<unsigned short, std::string> &errorPages = location.getErrorPages();
	std::ifstream											errorPageFile;
	std::stringstream										buff;
	std::map<unsigned short, std::string>::const_iterator	it;
	std::stringstream										ss;
	
	this->headerFields["content-type: "] = "text/html; charset=utf-8";
	it = errorPages.find(this->statusLine.statusCode);
	if (it != errorPages.end())
	{
		errorPageFile.open(it->second.c_str());
		if (errorPageFile)
		{
			buff << errorPageFile.rdbuf();
			this->responseBody = convertStringToOctets(buff.str());
		}
		errorPageFile.close();
	}
	ss << "<html>\r\n"
	   << "<head><title>" << this->statusLine.statusCode << " " << this->statusLine.reasonPhrase << "</title></head>\r\n"
	   << "<body>\r\n"
	   << "<center><h1>" << this->statusLine.statusCode << " " << this->statusLine.reasonPhrase << "</h1></center>\r\n"
	   << "<center><h2>" << this->statusDetails << "</h2></center>\r\n"
	   << "</html>\r\n";
	this->responseBody = convertStringToOctets(ss.str());
}

void HttpResponse::readRequestedFile(const std::string &targetResource, const stringmap_t& mimeExtensions)
{
	stringmap_t::const_iterator it;
	std::ifstream	stream(targetResource.c_str(), std::ios::binary);
	octets_t		contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
	std::string		extension;

    std::string::size_type lastDotPos = targetResource.rfind('.');
    std::string::size_type lastSlashPos = targetResource.rfind('/');
    if (lastDotPos != std::string::npos && (lastSlashPos == std::string::npos || lastDotPos > lastSlashPos))
        extension = targetResource.substr(lastDotPos + 1);
	else
		extension = "";
	it = mimeExtensions.find(extension);
	if (it != mimeExtensions.end())
		this->headerFields["content-type: "] = it->second;
	else
		this->headerFields["content-type: "] = "application/octet-stream";
	this->responseBody.insert(this->responseBody.end(), contents.begin(), contents.end());
	stream.close();
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
	std::stringstream		body;
	std::set<std::string>	strings;
	DIR*					dirPtr;

	this->headerFields["content-type: "] = "text/html; charset=utf-8";
	dirPtr = opendir(targetResource.c_str());
	body << "<html>\r\n"
		<< "<head><title>" << "Index of " << targetResource << "</title></head>\r\n"
		<< "<body>\r\n"
		<< "<h1>" << "Index of " << targetResource << "</h1>\r\n"
		<< "<hr><pre>\r\n";
	body << "<h2> 🏠 <a href=\"../\">"
		<< "Parent Directory"
		<< "</a></h2>";
	body << "<h2>"
		<< "      Name              "
		<< "    Date Modified       "
		<< "Size "
		<< "</h2>";
	for (dirent* dir = readdir(dirPtr); dir != NULL; dir = readdir(dirPtr))
	{
		std::stringstream	tempstream;
		std::string	path = targetResource + dir->d_name;
		struct stat	fileCheckBuff;
		if (stat(path.c_str(), &fileCheckBuff) < 0)
		{
			Logger::safeLog(DEBUG, RESPONSE, "Autoindexing failed due to failed system call", "");
			throw(std::exception());
		}
		#ifdef __APPLE__
			tm *curr_tm = std::gmtime(&(fileCheckBuff.st_mtimespec.tv_sec));
		#endif
		#ifdef __linux__
			tm *curr_tm = std::gmtime(&(fileCheckBuff.st_mtim.tv_sec));
		#endif
		char time[100];
		std::strftime(time, 100, "%a, %d %b %Y %H:%M:%S GMT", curr_tm);
		std::string	dname = dir->d_name;
		if (dname.size() > 28)
			dname = dname.substr(0, 25) + "...";
		if (dir->d_type == DT_DIR)
		{
			tempstream << " 📁 <a href=\""
			<< dir->d_name
			<< "/\">"
			<< dname
			<< "/"
			<< "</a>"
			<< std::left << std::setw(30 - dname.size()) << " "
			<< " "
			<< std::left << std::setw(30) << time
			<< " "
			<< std::right << std::setw(10) << "- "
			<< "\r\n";
		}
		else if (dir->d_type == DT_REG)
		{
			tempstream << " 📄 <a href=\""
			<< dir->d_name
			<< "\">"
			<< dname
			<< "" << "</a>"
			<< std::left << std::setw(31 - dname.size()) << " "
			<< " "
			<< std::left << std::setw(30) << time
			<< " ";
			double				size;
			std::string			strsize;
			size = fileCheckBuff.st_size;
			if (size > 1024ULL * 1024ULL * 1024ULL * 1024ULL)
			{
				size /= 1024ULL * 1024ULL * 1024ULL * 1024ULL;
				strsize = " TB";
				tempstream << std::right << std::setw(7) << std::fixed << std::setprecision(1)
				<< size << strsize
				<< "\r\n";
			}
			else if (size > 1024ULL * 1024ULL * 1024ULL)
			{
				size /= 1024ULL * 1024ULL * 1024ULL;
				strsize = " GB";
				tempstream << std::right << std::setw(7) << std::fixed << std::setprecision(1)
				<< size << strsize
				<< "\r\n";
			}
			else if (size > 1024ULL * 1024ULL)
			{
				size /= 1024ULL * 1024ULL;
				strsize = " MB";
				tempstream << std::right << std::setw(7) << std::fixed << std::setprecision(1)
				<< size << strsize
				<< "\r\n";
			}
			else if (size > 1024ULL)
			{
				size /= 1024ULL;
				strsize = " kB";
				tempstream << std::right << std::setw(7) << std::fixed << std::setprecision(1)
				<< size << strsize
				<< "\r\n";
			}
			else
			{
				strsize = "  B";
				tempstream << std::right << std::setw(7)
				<< size << strsize
				<< "\r\n";
			}
		}
		strings.insert(tempstream.str());
	}
	std::set<std::string>::iterator it;
	for (it = strings.begin(); it != strings.end(); ++it) {
		if ((*it).find(" 📁 <a href=\"./\">") != std::string::npos
			|| (*it).find(" 📁 <a href=\"../\">") != std::string::npos)
			continue ;
		body << *it;
	}
	body << "</pre><hr></body>\r\n";
	body << "</html>\r\n";
	this->responseBody = convertStringToOctets(body.str());
	closedir(dirPtr);
}

void	HttpResponse::deleteFile(const std::string& targetResource, const Location& location)
{
	if (targetResource.size() < location.getRoot().size())
	{
		Logger::safeLog(DEBUG, RESPONSE, "Attempt at deleting above server root ", "");
		throw (std::exception());
	}
	std::string	targetRoot = targetResource.substr(0, location.getRoot().size());
	if (targetRoot != location.getRoot())
	{
		Logger::safeLog(DEBUG, RESPONSE, "Attempt at deleting outside server root ", "");
		throw (std::exception());
	}
	int			validFile;
	struct stat	fileCheckBuff;
	validFile = stat(targetResource.c_str(), &fileCheckBuff);
	if (validFile < 0)
	{
		Logger::safeLog(DEBUG, RESPONSE, "Failed at deleting: ", targetResource);
		throw (std::exception());
	}
	if (S_ISDIR(fileCheckBuff.st_mode))
	{
		DIR*	dirPtr;
		dirPtr = opendir(targetResource.c_str());
		for (dirent* dir = readdir(dirPtr); dir != NULL; dir = readdir(dirPtr))
		{
			std::stringstream	tempstream;
			std::string	path = dir->d_name;
			if (path == "." || path == "..")
				continue;
			if (*targetResource.rbegin() == '/')
				path = targetResource + path;
			else
				path = targetResource + "/" + path;
			this->deleteFile(path, location);
		}
		closedir(dirPtr);
	}
	Logger::safeLog(DEBUG, RESPONSE, "Deleting: ", targetResource);
	if (std::remove(targetResource.c_str()))
	{
		Logger::safeLog(DEBUG, RESPONSE, "Failed at deleting: ", targetResource);
		throw (std::exception());
	}
}

void	HttpResponse::readReturnDirective(const Location &location)
{
	this->responseBody = convertStringToOctets(location.getReturnURLOrBody());
	this->headerFields["content-type: "] = "text/plain";
}

const octets_t		HttpResponse::prepareResponse(HttpRequest& request)
{
	if (request.getHasExpect())
	{
		Logger::safeLog(INFO, RESPONSE, "100 Continue", this->statusDetails);
		return (convertStringToOctets("HTTP/1.1 100 Continue\r\n\r\n"));
	}
	else
	{
		if (this->cgiStatus)
		{
			for (stringmap_t::iterator it = this->cgiHeaderFields.begin(); it != this->cgiHeaderFields.end(); it++)
				if (it->first == "status: ")
					this->statusLine.statusCode = atoi(it->second.c_str());
		}
		if (codeDict.find(this->statusLine.statusCode) == codeDict.end())
			this->codeDict[this->statusLine.statusCode] = "Undefined";
		this->statusLine.reasonPhrase = this->codeDict[this->statusLine.statusCode];

		if (!this->cgiStatus && (request.getRequestLine().method == "GET" || request.getRequestLine().method == "HEAD"))
		{
			if (this->statusLine.statusCode == 200 && request.getTargetIsDirectory() && !request.getLocation().getIsRedirect())
			{
				try
				{
					request.response.readDirectoryListing(request.getTargetResource());
				}
				catch(const std::exception& e)
				{
					this->statusLine.statusCode = 500;
					this->statusLine.reasonPhrase = this->codeDict[this->statusLine.statusCode];
					this->statusDetails = "Failed to index the directory";
					request.response.readErrorPage(request.getLocation());
				}
			}
			else if (this->statusLine.statusCode == 200 && !request.getLocation().getIsRedirect())
				request.response.readRequestedFile(request.getTargetResource(), request.getLocation().getMimeTypes().getMimeTypesDictInv());
			else if (request.getLocation().getIsRedirect() && (this->statusLine.statusCode < 300 || this->statusLine.statusCode > 308))
				request.response.readReturnDirective(request.getLocation());
			else if (this->statusLine.statusCode != 204 && this->statusLine.statusCode != 304) // this is in place so we don't read the error page for no reason
				request.response.readErrorPage(request.getLocation());
		}
		else if (!this->cgiStatus && request.getRequestLine().method == "POST")
		{
			// location header in prepareHeaders, status update here, if creation failed, 500 was raised
			if (this->statusLine.statusCode == 200 && !request.getLocation().getIsRedirect())
			{
				this->statusLine.statusCode = 201;
				this->statusLine.reasonPhrase = this->codeDict[this->statusLine.statusCode];
			}
			else if (request.getLocation().getIsRedirect() && (this->statusLine.statusCode < 300 || this->statusLine.statusCode > 308))
				request.response.readReturnDirective(request.getLocation());
			else if (this->statusLine.statusCode != 204 && this->statusLine.statusCode != 304) // this is in place so we don't read the error page for no reason
				request.response.readErrorPage(request.getLocation());
		}
		else if (!this->cgiStatus && request.getRequestLine().method == "DELETE")
		{
			if (this->statusLine.statusCode == 200 && !request.getLocation().getIsRedirect())
			{
				this->statusLine.statusCode = 204;
				this->statusLine.reasonPhrase = this->codeDict[this->statusLine.statusCode];
				try
				{
					request.response.deleteFile(request.getTargetResource(), request.getLocation());
				}
				catch(const std::exception& e)
				{
					this->statusLine.statusCode = 500;
					this->statusLine.reasonPhrase = this->codeDict[this->statusLine.statusCode];
					this->statusDetails = "Failed deletion of a targeted resource";
					request.response.readErrorPage(request.getLocation());
				}
			}
			else if (request.getLocation().getIsRedirect() && (this->statusLine.statusCode < 300 || this->statusLine.statusCode > 308))
				request.response.readReturnDirective(request.getLocation());
			else if (this->statusLine.statusCode != 204 && this->statusLine.statusCode != 304) // this is in place so we don't read the error page for no reason
				request.response.readErrorPage(request.getLocation());
		}
		else
		{
			this->responseBody.clear();
			this->responseBody = this->cgiBody;
			if (this->cgiStatus == CGI_ERROR)
				request.response.readErrorPage(request.getLocation());
		}
		//std::cout << this->cgiBody << std::endl;
		// std::cout << request.getRequestLine().requestTarget.absolutePath << std::endl;
		// std::cout << request.getRequestLine().requestTarget.query << std::endl;
		if (this->statusLine.statusCode == 204 || this->statusLine.statusCode == 304 || request.getRequestLine().method == "HEAD")
			this->responseBody.clear();
		request.response.buildResponseHeaders(request);
		request.response.buildCompleteResponse();
		octets_t message = request.response.getCompleteResponse();
		Logger::safeLog(INFO, RESPONSE, itoa(this->statusLine.statusCode) + " ", 
			this->statusLine.reasonPhrase + (this->statusDetails.size() ? ": " + this->statusDetails : ""));
		// std::cout << this->headerFields << std::endl;
		return (message);
	}
}

const octets_t &HttpResponse::getCompleteResponse() const
{
	return (this->completeResponse);
}

const octets_t&	HttpResponse::getMessage() const
{
	return(this->message);

}

void	HttpResponse::setMessage(const octets_t& message)
{
	this->message = message;
}

void	HttpResponse::eraseRangeMessage(size_t start, size_t end)
{
	if (start <= end && end <= this->message.size())
		this->message.erase(this->message.begin() + start, this->message.begin() + end);
}

bool	HttpResponse::getMessageTooLongForOneSend() const
{
	return (this->messageTooLongForOneSend);
}

void	HttpResponse::setMessageTooLongForOneSend(bool value)
{
	this->messageTooLongForOneSend = value;
}

int	HttpResponse::getCgiStatus(void)
{
	return(this->cgiStatus);
}

int	HttpResponse::getPostStatus(void)
{
	return(this->postStatus);
}


int	HttpResponse::getCgiPid(void)
{
	return(this->cgi_pid);
}

int	HttpResponse::getWfd(void)
{
	return(this->wfd);
}

int	HttpResponse::getRfd(void)
{
	return(this->rfd);
}

stringmap_t&	HttpResponse::getCgiHeaderFields(void)
{
	return(this->cgiHeaderFields);
}

octets_t&	HttpResponse::getCgiBody(void)
{
	return(this->cgiBody);
}

void	HttpResponse::setCgiStatus(int status)
{
	this->cgiStatus = status;
}

void	HttpResponse::setPostStatus(int status)
{
	this->postStatus = status;
}

void	HttpResponse::setCgiPid(int pid)
{
	this->cgi_pid = pid;
}

void	HttpResponse::setWfd(int fd)
{
	this->wfd = fd;
}

void	HttpResponse::setRfd(int fd)
{
	this->rfd = fd;
}
