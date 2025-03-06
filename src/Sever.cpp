#include "../inc/Server.hpp"

Server::Server()
{
	this->_port = 0;
	this->_host = 0;
	this->_servName = "";
	this->_root = "";
	this->_index = "";
	this->_autoIndex = false;
	this->_redirect = "";
	this->_maxBodySize = MAX_CONTENT_LENGTH;
	this->_listenFd = 0;
	this->initErrorPages();
}

Server::Server(const Server& copy)
{
	this->_servAddr = copy._servAddr;
	this->_locations = copy._locations;
	this->_port = copy._port;
	this->_host = copy._host;
	this->_servName = copy._servName;
	this->_root = copy._root;
	this->_index = copy._index;
	this->_autoIndex = copy._autoIndex;
	this->_redirect = copy._redirect;
	this->_maxBodySize = copy._maxBodySize;
	this->_listenFd = copy._listenFd;
	this->_errPag = copy._errPag;
}

Server& Server::operator=(const Server& copy)
{
	this->_servAddr = copy._servAddr;
	this->_locations = copy._locations;
	this->_port = copy._port;
	this->_host = copy._host;
	this->_servName = copy._servName;
	this->_root = copy._root;
	this->_index = copy._index;
	this->_autoIndex = copy._autoIndex;
	this->_redirect = copy._redirect;
	this->_maxBodySize = copy._maxBodySize;
	this->_listenFd = copy._listenFd;
	this->_errPag = copy._errPag;
	return *this;
}

Server::~Server() {}

void Server::setupServer()
{
	if ((_listenFd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		PrintApp::printEvent(RED_BOLD, FAILURE, "Failed to create socket: %s. Closing...", strerror(errno));
	int option_value = 1;
	setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(int));
	std::memset(&_servAddr, 0, sizeof(_servAddr));
	_servAddr.sin_family = AF_INET;
	_servAddr.sin_addr.s_addr = _host;
	_servAddr.sin_port = htons(_port);
	if (bind(_listenFd, (struct sockaddr*)&_servAddr, sizeof(_servAddr)) == -1)
		PrintApp::printEvent(RED_BOLD, FAILURE, "Failed to bind: %s. Closing...", strerror(errno));
}

void Server::setPort(const std::string& param)
{
	std::string p = checkToken(param);
	this->_port = static_cast<uint16_t>(parseInt(p, 1, 65636));
}

void Server::setHost(const std::string& param)
{
	std::string p = checkToken(param);
	if (p == "localhost")
		p = "127.0.0.1";
	struct in_addr addr;
	if (inet_pton(AF_INET, p.c_str(), &addr) != 1)
		throw Error("Invalid host address: " + p);
	this->_host = addr.s_addr;
}

void Server::setServerName(const std::string& servName)
{
	this->_servName = checkToken(servName);
}

void Server::setRoot(const std::string& root)
{
	std::string r = checkToken(root);
	if (r[0] != '/') {
		char dir[1024];
		getcwd(dir, 1024);
		r = std::string(dir) + "/" + r;
	}
	validateDirectory(r);
	this->_root = r;
}

void Server::setIndex(const std::string& index)
{
	this->_index = checkToken(index);
}

void Server::setAutoindex(const std::string& autoindex)
{
	std::string ai = checkToken(autoindex);
	if (ai == "on")
		this->_autoIndex = true;
	else if (ai == "off")
		this->_autoIndex = false;
	else
		throw Error("Autoindex must be 'on' or 'off', got: " + ai);
}

void Server::setRedirect(const std::string& redirect)
{
	this->_redirect = checkToken(redirect);
}

void Server::setClientMaxBodySize(const std::string& param)
{
	std::string p = checkToken(param);
	this->_maxBodySize = parseULong(p, INT_MAX);
}

void Server::setFd(int fd)
{
	this->_listenFd = fd;
}

void Server::setErrorPages(const std::vector<std::string>& param)
{
	if (param.empty())
		return;
	if (param.size() % 2 != 0)
		throw Error("Error pages require code-path pairs, got odd number of parameters");
	for (size_t i = 0; i < param.size(); i += 2)
	{
		std::string codeStr = param[i];
		std::string path = checkToken(param[i + 1]);
		short code = static_cast<short>(parseInt(codeStr, 400, 599));
		if (codeStr.size() != 3 || statusCodeString(code) == "Undefined")
			throw Error("Invalid error code (must be 400-599): " + codeStr);
		std::string fullPath = (path[0] == '/') ? path : this->_root + path;
		validateFile(fullPath);
		this->_errPag[code] = path;
	}
}

void Server::setLocation(const std::string& path, const std::vector<std::string>& param)
{
	if (param.empty())
		return;
	Location newLocation;
	bool flagMethods = false;
	bool flagAutoIndex = false;
	bool flagMaxSize = false;
	newLocation.setPath(path);
	for (size_t i = 0; i < param.size(); i++)
	{
		if (param[i] == "root" && i + 1 < param.size())
			handleRootLocation(param, i, newLocation);
		else if ((param[i] == "allow_methods" || param[i] == "methods") && i + 1 < param.size())
			handleAllowMethods(param, i, newLocation, flagMethods);
		else if (param[i] == "autoindex" && i + 1 < param.size())
			handleAutoindex(param, i, path, newLocation, flagAutoIndex);
		else if (param[i] == "index" && i + 1 < param.size())
			handleIndexLocation(param, i, newLocation);
		else if (param[i] == "return" && i + 1 < param.size())
			handleReturn(param, i, path, newLocation);
		else if (param[i] == "alias" && i + 1 < param.size())
			handleAlias(param, i, path, newLocation);
		else if (param[i] == "cgi_ext" && i + 1 < param.size())
			handleCgiExtension(param, i, newLocation);
		else if (param[i] == "cgi_path" && i + 1 < param.size())
			handleCgiPath(param, i, newLocation);
		else if (param[i] == "client_max_body_size" && i + 1 < param.size())
			handleMaxBodySize(param, i, newLocation, flagMaxSize);
		else if (param[i] == "redirect" && i + 1 < param.size())
			handleRedirect(param, i, path, newLocation);
		else
			throw Error("Unknown location directive: " + param[i]);
	}
	handleLocationDefaults(newLocation, flagMaxSize);
	int valid = isValidLocation(newLocation);
	handleLocationValidation(valid);
	if (path == "/cgi" && valid == 0)
	{
		const std::vector<std::string>& exts = newLocation.getCgiExtension();
		const std::vector<std::string>& paths = newLocation.getCgiPath();
		for (size_t i = 0; i < exts.size() && i < paths.size(); ++i)
		{
			std::string ext = exts[i];
			if (ext == ".py" || ext == "*.py")
			{
				std::string cgiPath = paths[i];
				if (cgiPath.find("python") != std::string::npos)
				    newLocation.extPath.insert(std::make_pair(".py", cgiPath));
			}
		}
	}
	this->_locations.push_back(newLocation);
}

void Server::initErrorPages()
{
	short codes[] = {301, 302, 400, 401, 402, 403, 404, 405, 406, 500, 501, 502, 503, 505};
	for (size_t i = 0; i < sizeof(codes) / sizeof(codes[0]); i++)
		_errPag[codes[i]] = "";
}

std::string Server::checkToken(const std::string& param)
{
	size_t pos = param.rfind(';');
	if (pos != param.size() - 1)
		throw Error("Parameter must end with ';', got: " + param);
	return param.substr(0, pos);
}

void Server::handleRootLocation(const std::vector<std::string>& param, size_t& i, Location& newLocation)
{
	if (!newLocation.getRootLocation().empty())
		throw Error("Duplicate root directive in location");
	std::string root = checkToken(param[++i]);
	std::string rootLocation = (ConfigFile::getTypePath(root) == 2) ? root : this->_root + root;
	newLocation.setRootLocation(rootLocation);
}

void Server::handleIndexLocation(const std::vector<std::string>& param, size_t& i, Location& newLocation)
{
	if (!newLocation.getIndexLocation().empty())
		throw Error("Duplicate index directive in location");
	newLocation.setIndexLocation(checkToken(param[++i]));
}

void Server::handleAutoindex(const std::vector<std::string>& param, size_t& i, const std::string& path, Location& newLocation, bool& flagAutoIndex)
{
	if (path == "/cgi")
		throw Error("Autoindex not allowed for CGI location");
	if (flagAutoIndex)
		throw Error("Duplicate autoindex directive in location");
	newLocation.setAutoindex(checkToken(param[++i]));
	flagAutoIndex = true;
}

void Server::handleRedirect(const std::vector<std::string>& param, size_t& i, const std::string& path, Location& newLocation)
{
	if (path == "/cgi")
		throw Error("Redirect not allowed for CGI location");
	if (!newLocation.getRedirect().empty())
		throw Error("Duplicate redirect directive in location");
	newLocation.setRedirect(checkToken(param[++i]));
}

void Server::handleReturn(const std::vector<std::string>& param, size_t& i, const std::string& path, Location& newLocation)
{
	if (path == "/cgi")
		throw Error("Return not allowed for CGI location");
	if (!newLocation.getReturn().empty())
		throw Error("Duplicate return directive in location");
	newLocation.setReturn(checkToken(param[++i]));
}

void Server::handleAlias(const std::vector<std::string>& param, size_t& i, const std::string& path, Location& newLocation)
{
	if (path == "/cgi")
		throw Error("Alias not allowed for CGI location");
	if (!newLocation.getAlias().empty())
		throw Error("Duplicate alias directive in location");
	newLocation.setAlias(checkToken(param[++i]));
}

void Server::handleMaxBodySize(const std::vector<std::string>& param, size_t& i, Location& newLocation, bool& flagMaxSize)
{
	if (flagMaxSize)
		throw Error("Duplicate client_max_body_size directive in location");
	newLocation.setMaxBodySize(checkToken(param[++i]));
	flagMaxSize = true;
}

void Server::handleAllowMethods(const std::vector<std::string>& param, size_t& i, Location& newLocation, bool& flagMethods)
{
	if (flagMethods)
		throw Error("Duplicate allow_methods directive in location");
	std::vector<std::string> methods;
	size_t j = i;
	while (++j < param.size() && param[j].find(";") == std::string::npos)
		methods.push_back(param[j]);
	if (j >= param.size())
		throw Error("Allow_methods missing semicolon");
	methods.push_back(checkToken(param[j]));
	newLocation.setMethods(methods);
	i = j;
	flagMethods = true;
}

void Server::handleCgiExtension(const std::vector<std::string>& param, size_t& i, Location& newLocation)
{
	std::vector<std::string> extensions;
	size_t j = i;
	while (++j < param.size() && param[j].find(";") == std::string::npos)
		extensions.push_back(param[j]);
	if (j >= param.size())
		throw Error("CGI extension missing semicolon");
	extensions.push_back(checkToken(param[j]));
	newLocation.setCgiExtension(extensions);
	i = j;
}

void Server::handleCgiPath(const std::vector<std::string>& param, size_t& i, Location& newLocation)
{
	std::vector<std::string> paths;
	size_t j = i;
	while (++j < param.size() && param[j].find(";") == std::string::npos)
		paths.push_back(param[j]);
	if (j >= param.size())
		throw Error("CGI path missing semicolon");
	std::string lastPath = checkToken(param[j]);
	if (lastPath.find("/python") == std::string::npos)
		throw Error("CGI path must include 'python': " + lastPath);
	paths.push_back(lastPath);
	newLocation.setCgiPath(paths);
	i = j;
}

const std::vector<Location>& Server::getLocations()
{
	return this->_locations;
}

const uint16_t& Server::getPort()
{
	return this->_port;
}

const in_addr_t& Server::getHost()
{
	return this->_host;
}

const std::string& Server::getServerName()
{
	return this->_servName;
}

const std::string& Server::getRoot()
{
	return this->_root;
}

const std::string& Server::getIndex()
{
	return this->_index;
}

const bool& Server::getAutoindex()
{
	return this->_autoIndex;
}

const std::string& Server::getRedirect()
{
	return this->_redirect;
}

const size_t& Server::getClientMaxBodySize()
{
	return this->_maxBodySize;
}

int Server::getFd()
{
	return this->_listenFd;
}

const std::map<short, std::string>& Server::getErrorPages()
{
	return this->_errPag;
}

const std::vector<Location>::iterator Server::getLocationKey(std::string key)
{
	for (std::vector<Location>::iterator it = _locations.begin(); it != _locations.end(); ++it)
		if (it->getPath() == key) return it;
	throw Error("Path to location not found: " + key);
}

const std::string& Server::getPathErrorPage(short key)
{
	std::map<short, std::string>::iterator it = _errPag.find(key);
	if (it == _errPag.end())
		throw Error("Error page not found for code: " + toString(key));
	return it->second;
}

void Server::handleLocationDefaults(Location& newLocation, bool flagMaxSize)
{
	if (newLocation.getPath() != "/cgi" && newLocation.getIndexLocation().empty())
		newLocation.setIndexLocation(this->_index);
	if (!flagMaxSize)
		newLocation.setMaxBodySize(this->_maxBodySize);
}

void Server::handleLocationValidation(int valid)
{
	if (valid == 1)
		throw Error("Invalid CGI configuration");
	if (valid == 2)
		throw Error("Invalid path configuration");
	if (valid == 3)
		throw Error("Invalid redirection file");
	if (valid == 4)
		throw Error("Invalid alias file");
}

int Server::isValidLocation(Location& location) const
{
	return (location.getPath() == "/cgi") ? isValidCgiLocation(location) : isValidRegularLocation(location);
}

int Server::isValidRegularLocation(Location& location) const
{
	if (location.getPath()[0] != '/')
		return 2;
	if (location.getRootLocation().empty())
		location.setRootLocation(this->_root);
	if (ConfigFile::CheckFile(location.getRootLocation() + location.getPath() + "/", location.getIndexLocation()))
		return 5;
	if (!location.getReturn().empty() && ConfigFile::CheckFile(location.getRootLocation(), location.getReturn()))
		return 3;
	if (!location.getAlias().empty() && ConfigFile::CheckFile(location.getRootLocation(), location.getAlias()))
		return 4;
	return 0;
}

bool Server::isValidHost(std::string host) const
{
	struct sockaddr_in sockaddr;
	return inet_pton(AF_INET, host.c_str(), &(sockaddr.sin_addr)) != 0;
}

int Server::isValidCgiLocation(Location& location) const
{
	if (location.getCgiPath().empty() || location.getCgiExtension().empty() || location.getIndexLocation().empty())
		return 1;
	if (ConfigFile::checkAccessFile(location.getIndexLocation(), 4) < 0)
	{
		std::string path = location.getRootLocation() + location.getPath() + "/" + location.getIndexLocation();
		if (ConfigFile::getTypePath(path) != 1)
		{
			std::string root = getcwd(NULL, 0);
			location.setRootLocation(root);
			path = root + location.getPath() + "/" + location.getIndexLocation();
		}
		if (path.empty() || ConfigFile::getTypePath(path) != 1 || ConfigFile::checkAccessFile(path, 4) < 0)
			return 1;
	}
	if (location.getCgiPath().size() != location.getCgiExtension().size())
		return 1;
	for (std::vector<std::string>::const_iterator it = location.getCgiPath().begin(); it != location.getCgiPath().end(); ++it)
	{
		if (ConfigFile::getTypePath(*it) < 0)
			return 1;
	}
	for (std::vector<std::string>::const_iterator it = location.getCgiExtension().begin(); it != location.getCgiExtension().end(); ++it)
	{
		if (*it != ".py" && *it != "*.py")
			return 1;
	}
	return 0;
}

bool Server::isValidErrorPages()
{
	for (std::map<short, std::string>::const_iterator it = _errPag.begin(); it != _errPag.end(); ++it)
	{
		if (it->first < 100 || it->first > 599) return false;
		if (!it->second.empty() && (ConfigFile::checkAccessFile(getRoot() + it->second, 0) < 0 || ConfigFile::checkAccessFile(getRoot() + it->second, 4) < 0)) return false;
	}
	return true;
}

bool Server::checkLocation() const
{
	if (_locations.size() < 2) return false;
	for (std::vector<Location>::const_iterator it1 = _locations.begin(); it1 != _locations.end() - 1; ++it1)
	{
		for (std::vector<Location>::const_iterator it2 = it1 + 1; it2 != _locations.end(); ++it2)
			if (it1->getPath() == it2->getPath()) return true;
	}
	return false;
}

std::string Server::statusCodeString(short statusCode)
{
	std::map<short, std::string> statusCodes;
	statusCodes[100] = "Continue";
	statusCodes[101] = "Switching Protocol";
	statusCodes[200] = "OK";
	statusCodes[201] = "Created";
	statusCodes[202] = "Accepted";
	statusCodes[203] = "Non-Authoritative Information";
	statusCodes[204] = "No Content";
	statusCodes[205] = "Reset Content";
	statusCodes[206] = "Partial Content";
	statusCodes[300] = "Multiple Choice";
	statusCodes[301] = "Moved Permanently";
	statusCodes[302] = "Moved Temporarily";
	statusCodes[303] = "See Other";
	statusCodes[304] = "Not Modified";
	statusCodes[307] = "Temporary Redirect";
	statusCodes[308] = "Permanent Redirect";
	statusCodes[400] = "Bad Request";
	statusCodes[401] = "Unauthorized";
	statusCodes[403] = "Forbidden";
	statusCodes[404] = "Not Found";
	statusCodes[405] = "Method Not Allowed";
	statusCodes[406] = "Not Acceptable";
	statusCodes[407] = "Proxy Authentication Required";
	statusCodes[408] = "Request Timeout";
	statusCodes[409] = "Conflict";
	statusCodes[410] = "Gone";
	statusCodes[411] = "Length Required";
	statusCodes[412] = "Precondition Failed";
	statusCodes[413] = "Payload Too Large";
	statusCodes[414] = "URI Too Long";
	statusCodes[415] = "Unsupported Media Type";
	statusCodes[416] = "Requested Range Not Satisfiable";
	statusCodes[417] = "Expectation Failed";
	statusCodes[418] = "I'm a teapot";
	statusCodes[421] = "Misdirected Request";
	statusCodes[425] = "Too Early";
	statusCodes[426] = "Upgrade Required";
	statusCodes[428] = "Precondition Required";
	statusCodes[429] = "Too Many Requests";
	statusCodes[431] = "Request Header Fields Too Large";
	statusCodes[451] = "Unavailable for Legal Reasons";
	statusCodes[500] = "Internal Server Error";
	statusCodes[501] = "Not Implemented";
	statusCodes[502] = "Bad Gateway";
	statusCodes[503] = "Service Unavailable";
	statusCodes[504] = "Gateway Timeout";
	statusCodes[505] = "HTTP Version Not Supported";
	statusCodes[506] = "Variant Also Negotiates";
	statusCodes[507] = "Insufficient Storage";
	statusCodes[510] = "Not Extended";
	statusCodes[511] = "Network Authentication Required";
	std::map<short, std::string>::iterator it = statusCodes.find(statusCode);
	return (it != statusCodes.end()) ? it->second : "Undefined";
}

void Server::validateFile(const std::string& path)
{
	if (ConfigFile::getTypePath(path) != 1)
		throw Error("Not a file: " + path);
	if (ConfigFile::checkAccessFile(path, 0) == -1 || ConfigFile::checkAccessFile(path, 4) == -1)
		throw Error("File not accessible: " + path);
}

void Server::validateDirectory(const std::string& path)
{
	if (ConfigFile::getTypePath(path) != 2)
		throw Error("Not a directory: " + path);
}

int Server::parseInt(const std::string& str, int min, int max)
{
	char* end;
	long val = std::strtol(str.c_str(), &end, 10);
	if (end == str.c_str() || *end != '\0' || val < min || val > max)
		throw Error("Invalid integer value: " + str);
	return static_cast<int>(val);
}

unsigned long Server::parseULong(const std::string& str, unsigned long max)
{
	char* end;
	unsigned long val = std::strtoul(str.c_str(), &end, 10);
	if (end == str.c_str() || *end != '\0' || val > max)
		throw Error("Invalid unsigned long value: " + str);
	return val;
}
