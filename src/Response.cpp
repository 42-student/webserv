#include "../inc/Response.hpp"

Response::Response()
{
	_targetFile = "";
	_respBody = "";
	_loc = "";
	_body.clear();
	_bodyLen = 0;
	_cgi = 0;
	_cgiRespLen = 0;
	_autoIndex = 0;
	_code = 0;
	responseContent = "";
	_mimeType["default"] = "text/html";
	_mimeType[".html"] = "text/html";
	_mimeType[".jpg"] = "image/jpg";
	_mimeType[".jpeg"] = "image/jpeg";
	_mimeType[".png"] = "image/png";
	_mimeType[".ico"] = "image/x-icon";
	_mimeType[".svg"] = "image/svg+xml";
	_mimeType[".bmp"] = "image/bmp";
}

Response::~Response() {}

Response::Response(Request &req) : request(req)
{
	_targetFile = "";
	_respBody = "";
	_loc = "";
	_body.clear();
	_bodyLen = 0;
	_cgi = 0;
	_cgiRespLen = 0;
	_autoIndex = 0;
	_code = 0;
	responseContent = "";
}

void Response::buildResponse()
{
	if (reqError() || buildBody())
		buildErrorBody();
	if (_cgi)
		return;
	else if (_autoIndex)
	{
		std::cout << "Auto index " << std::endl;
		if (buildHtmlIndex(_targetFile, _body, _bodyLen))
		{
			_code = 500;
			buildErrorBody();
		}
		else
			_code = 200;
		_respBody.insert(_respBody.begin(), _body.begin(), _body.end());
	}
	setStatusLine();
	setHeaders();
	if (request.getHttpMethod() == GET || _code != 200)
		responseContent.append(_respBody);
}

bool Response::reqError()
{
	if (request.errorCodes())
	{
		_code = request.errorCodes();
		return (1);
	}
	return (0);
}

int Response::buildBody()
{
	if (isBodySizeExceeded())
	{
		_code = 413;
		return 1;
	}
	if (handleTarget())
		return 1;
	if (_code)
		return 0;
	if (request.getHttpMethod() == GET)
	{
		if (handleGetMethod())
			return 1;
	}
	else if (request.getHttpMethod() == POST)
	{
		if (handlePostMethod())
			return 1;
	}
	else if (request.getHttpMethod() == DELETE)
	{
		if (handleDeleteMethod())
			return 1;
	}
	_code = 200;
	return 0;
}

void Response::buildErrorBody()
{
	if (!_serv.getErrorPages().count(_code) || _serv.getErrorPages().at(_code).empty() ||
		request.getHttpMethod() == DELETE || request.getHttpMethod() == POST)
		setServerDefaultErrorPages();
	else
	{
		if (_code >= 400 && _code < 500)
		{
			_loc = _serv.getErrorPages().at(_code);
			if (_loc[0] != '/')
				_loc.insert(_loc.begin(), '/');
			_code = 302;
		}	
		_targetFile = _serv.getRoot() + _serv.getErrorPages().at(_code);
		short oldcode = _code;
		if (readFile())
		{
			_code = oldcode;
			_respBody = getErrorPage(_code);
		}
	}
}

int Response::buildHtmlIndex(std::string &dirName, std::vector<uint8_t> &body, size_t &bodyLen)
{
	struct dirent *entityStruct;
	DIR *directory;
	std::string dirListPage;
	directory = opendir(dirName.c_str());
	if (directory == NULL)
	{
		std::cerr << "opendir failed" << std::endl;
		return (1);
	}
	dirListPage.append("<html>\n");
	dirListPage.append("<head>\n");
	dirListPage.append("<title> Index of");
	dirListPage.append(dirName);
	dirListPage.append("</title>\n");
	dirListPage.append("</head>\n");
	dirListPage.append("<body >\n");
	dirListPage.append("<h1> Index of " + dirName + "</h1>\n");
	dirListPage.append("<table style=\"width:80%; font-size: 15px\">\n");
	dirListPage.append("<hr>\n");
	dirListPage.append("<th style=\"text-align:left\"> File Name </th>\n");
	dirListPage.append("<th style=\"text-align:left\"> Last Modification  </th>\n");
	dirListPage.append("<th style=\"text-align:left\"> File Size </th>\n");	
	struct stat fileStat;
	std::string filePath;
	while ((entityStruct = readdir(directory)) != NULL)
	{
		if (strcmp(entityStruct->d_name, ".") == 0)
			continue;
		filePath = dirName + entityStruct->d_name;
		stat(filePath.c_str(), &fileStat);
		dirListPage.append("<tr>\n");
		dirListPage.append("<td>\n");
		dirListPage.append("<a href=\"");
		dirListPage.append(entityStruct->d_name);
		if (S_ISDIR(fileStat.st_mode))
			dirListPage.append("/");
		dirListPage.append("\">");
		dirListPage.append(entityStruct->d_name);
		if (S_ISDIR(fileStat.st_mode))
			dirListPage.append("/");
		dirListPage.append("</a>\n");
		dirListPage.append("</td>\n");
		dirListPage.append("<td>\n");
		dirListPage.append(ctime(&fileStat.st_mtime));
		dirListPage.append("</td>\n");
		dirListPage.append("<td>\n");
		if (!S_ISDIR(fileStat.st_mode))
			dirListPage.append(toString(fileStat.st_size));
		dirListPage.append("</td>\n");
		dirListPage.append("</tr>\n");
	}
	dirListPage.append("</table>\n");
	dirListPage.append("<hr>\n");
	dirListPage.append("</body>\n");
	dirListPage.append("</html>\n");
	body.insert(body.begin(), dirListPage.begin(), dirListPage.end());
	bodyLen = body.size();
	return (0);
}

bool Response::isBodySizeExceeded()
{
	return (request.getBody().length() > _serv.getClientMaxBodySize());
}

int Response::handleTarget()
{
	std::string locationKey;
	getLocationMatch(request.getPath(), _serv.getLocations(), locationKey);	
	if (!locationKey.empty())
	{
		Location targetLocation = *_serv.getLocationKey(locationKey);
		if (isMethodNotAllowed(targetLocation))
		{
			std::cout << "METHOD NOT ALLOWED \n";
			return 1;
		}
		if (isRequestBodySizeExceeded(request.getBody(), targetLocation))
		{
			_code = 413;
			return 1;
		}
		if (checkLocationReturn(targetLocation))
			return 1;
		if (checkLocationRedirect(targetLocation))
			return 1;
		if (isCgiPath(targetLocation.getPath()))
			return handleCgi(locationKey);
		if (!targetLocation.getAlias().empty())
			replaceAlias(targetLocation, request, _targetFile);
		else
			appendRoot(targetLocation, request, _targetFile);
		if (!targetLocation.getCgiExtension().empty())
		{
			if (isCgiExtension(_targetFile, targetLocation))
				return controllerCgiTemp(locationKey);
		}
		if (isDirectory(_targetFile))
		{
			if (_targetFile[_targetFile.length() - 1] != '/')
			{
				_code = 301;
				_loc = request.getPath() + "/";
				return 1;
			}	
			return handleIndexLocation(targetLocation.getIndexLocation(), targetLocation.getAutoindex());
		}
	}
	else
		return handleNonLocation(_serv.getRoot(), request);
	return 0;
}

bool Response::isMethodNotAllowed(Location &location)
{
	return isAllowedMethod(request.getHttpMethod(), location, _code);
}

bool Response::isAllowedMethod(int method, Location &location, short &code)
{
	std::vector<short> methods = location.getMethods();
	if ((method == 0 && !methods[0]) || (method == 1 && !methods[1]) ||
		(method == 2 && !methods[2])) {
		code = 405;
		return (1);
	}
	return (0);
}

bool Response::isRequestBodySizeExceeded(const std::string &body, const Location &location)
{
	return (body.length() > location.getMaxBodySize());
}

bool Response::checkLocationReturn(Location &location)
{
	return checkReturn(location, _code, this->_loc);
}

bool Response::checkReturn(Location &loc, short &code, std::string &location)
{
	if (loc.getReturn().empty())
		return (0);
	code = 301;
	location = loc.getReturn();
	if (location[0] != '/')
		location.insert(location.begin(), '/');
	return (1);
}

bool Response::checkLocationRedirect(Location &location)
{
	return checkRedirect(location, _code, this->_loc);
}

bool Response::checkRedirect(Location &loc, short &code, std::string &location)
{
	if (loc.getRedirect().empty())
		return (0);
	code = 301;
	location = loc.getRedirect();
	return (1);
}

bool Response::isCgiPath(const std::string &path)
{
	return (path.find("cgi") != std::string::npos);
}

int Response::handleCgi(std::string &locationKey)
{
	std::string path = this->request.getPath();
	std::string exten;
	size_t pos = 0;
	if (!isValidPath(path, locationKey, pos))
		return (1);
	if (!isValidExtension(path))
		return (1);
	if (!isValidFileType(path))
		return (1);
	if (!isFileAllowed(request, locationKey))
		return (1);
	if (!(initializeCgi(path, locationKey)))
		return (1);
	return (0);
}

bool Response::isValidPath(std::string &path, std::string &locationKey, size_t &pos)
{
	if (!path.empty() && path[0] == '/')
		path.erase(0, 1);
	if (path == "cgi")
		path += "/" + _serv.getLocationKey(locationKey)->getIndexLocation();
	else if (path == "cgi/")
		path.append(_serv.getLocationKey(locationKey)->getIndexLocation());
	pos = path.find(".");
	if (pos == std::string::npos)
	{
		_code = 501;
		return false;
	}
	return true;
}

bool Response::isValidExtension(std::string &path)
{
	std::string exten = path.substr(path.find("."));
	if (exten != ".py")
	{
		_code = 501;
		return false;
	}
	return true;
}

bool Response::isValidFileType(std::string &path)
{
	if (ConfigFile::getTypePath(path) != 1)
	{
		_code = 404;
		return false;
	}	
	if (ConfigFile::checkAccessFile(path, 1) == -1 || ConfigFile::checkAccessFile(path, 3) == -1)
	{
		_code = 403;
		return false;
	}
	return true;
}

bool Response::isFileAllowed(Request &request, std::string &locationKey)
{
	if (isAllowedMethod(request.getHttpMethod(), *_serv.getLocationKey(locationKey), _code))
		return false;
	return true;
}

bool Response::initializeCgi(std::string &path, std::string &locationKey)
{
	cgiObj.clear();
	cgiObj.setCgiPath(path);
	_cgi = 1;
	if (pipe(_cgiFd) < 0)
	{
		_code = 500;
		return (true);
	}
	cgiObj.initEnv(request, _serv.getLocationKey(locationKey));
	cgiObj.execute(this->_code);
	return (false);
}

void Response::replaceAlias(Location &location, Request &request, std::string &_targetFile)
{
	_targetFile = combinePaths(location.getAlias(), request.getPath().substr(location.getPath().length()), "");
}

void Response::appendRoot(Location &location, Request &request, std::string &_targetFile)
{
	_targetFile = combinePaths(location.getRootLocation(), request.getPath(), "");
}

std::string Response::combinePaths(std::string p1, std::string p2, std::string p3)
{
	if (p1[p1.length() - 1] == '/' && (!p2.empty() && p2[0] == '/'))
		p2.erase(0, 1);
	if (p1[p1.length() - 1] != '/' && (!p2.empty() && p2[0] != '/'))
		p1.insert(p1.end(), '/');
	if (p2[p2.length() - 1] == '/' && (!p3.empty() && p3[0] == '/'))
		p3.erase(0, 1);
	if (p2[p2.length() - 1] != '/' && (!p3.empty() && p3[0] != '/'))
		p2.insert(p1.end(), '/');
	return (p1 + p2 + p3);
}

int Response::controllerCgiTemp(std::string &locationKey)
{
	std::string path;
	path = _targetFile;
	cgiObj.clear();
	cgiObj.setCgiPath(path);
	_cgi = 1;
	if (pipe(_cgiFd) < 0)
	{
		_code = 500;
		return (1);
	}
	cgiObj.initEnvCgi(request, _serv.getLocationKey(locationKey));
	cgiObj.execute(this->_code);
	return (0);
}

bool Response::isDirectory(std::string path)
{
	struct stat file_stat;
	if (stat(path.c_str(), &file_stat) != 0)
		return (false);
	return (S_ISDIR(file_stat.st_mode));
}

bool Response::handleIndexLocation(const std::string &indexLocation, bool autoindex)
{
	if (!indexLocation.empty())
		_targetFile += indexLocation;
	else
		_targetFile += _serv.getIndex();
	if (!fileExists(_targetFile))
	{
		if (autoindex)
		{
			_targetFile.erase(_targetFile.find_last_of('/') + 1);
			_autoIndex = true;
			return false;
		}
		else
		{
			_code = 403;
			return true;
		}
	}
	if (isDirectory(_targetFile))
	{
		_code = 301;
		if (!indexLocation.empty())
			_loc = combinePaths(request.getPath(), indexLocation, "");
		else
			_loc = combinePaths(request.getPath(), _serv.getIndex(), "");
		if (_loc[_loc.length() - 1] != '/')
			_loc.insert(_loc.end(), '/');
		return true;
	}	
	return false;
}

bool Response::fileExists(const std::string &f)
{
	std::ifstream file(f.c_str());
	return (file.good());
}

bool Response::handleNonLocation(const std::string &root, Request &request)
{
	_targetFile = combinePaths(root, request.getPath(), "");
	if (isDirectory(_targetFile))
	{
		if (_targetFile[_targetFile.length() - 1] != '/')
		{
			_code = 301;
			_loc = request.getPath() + "/";
			return true;
		}
		_targetFile += _serv.getIndex();
		if (!fileExists(_targetFile))
		{
			_code = 403;
			return true;
		}
		if (isDirectory(_targetFile))
		{
			_code = 301;
			_loc = combinePaths(request.getPath(), _serv.getIndex(), "");
			if (_loc[_loc.length() - 1] != '/')
				_loc.insert(_loc.end(), '/');
			return true;
		}
	}
	return false;
}

void Response::setRequest(Request &req)
{
	request = req;
}

void Response::setServer(Server &serv)
{
	this->_serv = serv;
}

void Response::setStatusLine()
{
	responseContent.append("HTTP/1.1 " + toString(_code) + " ");
	responseContent.append(Server::statusCodeString(_code));
	responseContent.append("\r\n");
}

void Response::setHeaders()
{
	contentType();
	contentLength();
	connection();
	server();
	locations();
	date();
	responseContent.append("\r\n");
}

void Response::setServerDefaultErrorPages()
{
	_respBody = getErrorPage(_code);
}

void Response::setErrorResponse(short code)
{
	responseContent = "";
	this->_code = code;
	_respBody = "";
	buildErrorBody();
	setStatusLine();
	setHeaders();
	responseContent.append(_respBody);
}

void Response::setCgiState(int state)
{
	_cgi = state;
}

void Response::getLocationMatch(std::string &path, std::vector<Location> locations, std::string &locationKey)
{
	size_t bigMatch = 0;
	for (std::vector<Location>::const_iterator it = locations.begin(); it != locations.end(); ++it)
	{
		if (path.find(it->getPath()) == 0)
		{
			if (it->getPath() == "/" || path.length() == it->getPath().length() || path[it->getPath().length()] == '/')
			{
				if (it->getPath().length() > bigMatch)
				{
					bigMatch = it->getPath().length();
					locationKey = it->getPath();
				}
			}
		}
	}
}

std::string Response::getRes()
{
	return (responseContent);
}

size_t Response::getLen() const
{
	return (responseContent.length());
}

int Response::getCode() const
{
	return (_code);
}

int Response::getCgiState()
{
	return (_cgi);
}

std::string Response::getErrorPage(short statusCode)
{
	return ("<html>\r\n<head><title>" + toString(statusCode) + " " + Server::statusCodeString(statusCode) + " </title></head>\r\n" + "<body>\r\n" + "<center><h1>" + toString(statusCode) + " " + Server::statusCodeString(statusCode) + "</h1></center>\r\n");
}

std::string Response::getMimeType(std::string extension) const
{
	std::map<std::string, std::string>::const_iterator it = _mimeType.find(extension);
	if (it != _mimeType.end())
	{
		return it->second;
	}
	return _mimeType.find("default")->second;
}

bool Response::hasMimeType(std::string &extension) const
{
	return _mimeType.find(extension) != _mimeType.end();
}

bool Response::isBoundaryLine(const std::string &line, const std::string &boundary)
{
	return (line.compare("--" + boundary + "\r") == 0);
}

bool Response::isEndBoundaryLine(const std::string &line, const std::string &boundary)
{
	return (line.compare("--" + boundary + "--\r") == 0);
}

bool Response::isCgiExtension(const std::string &_targetFile, const Location &location)
{
	return (_targetFile.rfind(location.getCgiExtension()[0]) != std::string::npos);
}

bool Response::handleTgt()
{
	if (_cgi || _autoIndex)
		return true;
	return false;
}

bool Response::handleGetMethod()
{
	if (readFile())
		return true;
	return false;
}

bool Response::handlePostMethod()
{
	if (fileExists(_targetFile) && request.getHttpMethod() == POST)
	{
		_code = 204;
		return false;
	}
	std::ofstream file(_targetFile.c_str(), std::ios::binary);
	if (file.fail())
	{
		_code = 404;
		return true;
	}
	if (request.getMultiformFlag())
	{
		std::string body = request.getBody();
		body = removeBoundary(body, request.getBoundary());
		file.write(body.c_str(), body.length());
	}
	else
		file.write(request.getBody().c_str(), request.getBody().length());
	return false;
}

bool Response::handleDeleteMethod()
{
	if (!fileExists(_targetFile))
	{
		_code = 404;
		return true;
	}
	if (remove(_targetFile.c_str()) != 0)
	{
		_code = 500;
		return true;
	}
	return false;
}

int Response::readFile()
{
	std::ifstream file(_targetFile.c_str());
	if (file.fail())
	{
	  _code = 404;
	  return (1);
	}
	std::ostringstream ss;
	ss << file.rdbuf();
	_respBody = ss.str();
	return (0);
}

void Response::contentType()
{
	responseContent.append("Content-Type: ");
	std::string::size_type dotPos = _targetFile.rfind(".");
	std::string extension = (dotPos != std::string::npos) ? _targetFile.substr(dotPos) : "";
	std::string mimeType = (dotPos != std::string::npos && _code == 200) ? this->getMimeType(extension) : this->getMimeType("default");
	responseContent.append(mimeType);
	responseContent.append("\r\n");
}

void Response::contentLength()
{
	std::stringstream ss;
	ss << _respBody.length();
	responseContent.append("Content-Length: ");
	responseContent.append(ss.str());
	responseContent.append("\r\n");
}

void Response::connection()
{
	if (request.getHeader("connection") == "keep-alive")
		responseContent.append("Connection: keep-alive\r\n");
}

void Response::server()
{
	responseContent.append("Server: miau\r\n");
}

void Response::locations()
{
	if (_loc.length())
		responseContent.append("Location: " + _loc + "\r\n");
}

void Response::date()
{
	char date[1000];
	time_t now = time(0);
	struct tm tm = *gmtime(&now);
	strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S %Z", &tm);
	responseContent.append("Date: ");
	responseContent.append(date);
	responseContent.append("\r\n");
}

std::string Response::removeBoundary(std::string &body, const std::string &boundary)
{
	std::string newBody;
	std::string buffer;
	std::string filename;
	bool isBoundary = false;
	bool isContent = false;
	if (body.find("--" + boundary) != std::string::npos && body.find("--" + boundary + "--") != std::string::npos)
	{
		std::string::size_type pos = 0;
		std::string::size_type prevPos = 0;
		while ((pos = body.find("\n", pos)) != std::string::npos)
		{
			buffer = body.substr(prevPos, pos - prevPos);	
			if (isEndBoundaryLine(buffer, boundary))
			{
				isContent = true;
				isBoundary = false;
			}
			if (isBoundaryLine(buffer, boundary))
			{
				isBoundary = true;
				filename = extractFilename(buffer);
			}
			if (isBoundary)
			{
				if (!filename.empty() && buffer.compare("\r") == 0)
				{
					isBoundary = false;
					isContent = true;
				}
			}
			else if (isContent)
			{
				if (isBoundaryLine(buffer, boundary))
					isBoundary = true;
				else if (isEndBoundaryLine(buffer, boundary))
					break;
				else
					newBody += (buffer + "\n");
			}
			prevPos = ++pos;
		}
	}
	body = newBody;
	return body;
}

std::string Response::extractFilename(const std::string &line)
{
	std::string filename;
	size_t start = line.find("filename=\"");
	if (start != std::string::npos)
	{
		size_t end = line.find("\"", start + 10);
		if (end != std::string::npos)
			filename = line.substr(start + 10, end - (start + 10));
	}
	return filename;
}

void Response::cutRes(size_t i)
{
	responseContent = responseContent.substr(i);
}

void Response::clear()
{
	_targetFile.clear();
	_respBody.clear();
	_loc.clear();
	_body.clear();
	_bodyLen = 0;
	_cgi = 0;
	_cgiRespLen = 0;
	_autoIndex = 0;
	_code = 0;
	responseContent.clear();
}
