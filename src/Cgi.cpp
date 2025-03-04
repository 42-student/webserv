#include "../inc/Cgi.hpp"

Cgi::Cgi()
{
	this->_cgiPid = -1;
	this->_exitStatus = 0;
	this->_cgiPath = "";
	this->_chEnv = NULL;
	this->_av = NULL;
}

Cgi::Cgi(std::string path)
{
	this->_cgiPid = -1;
	this->_exitStatus = 0;
	this->_cgiPath = path;
	this->_chEnv = NULL;
	this->_av = NULL;
}

Cgi::Cgi(const Cgi &copy)
{
	this->_env = copy._env;
	this->_chEnv = copy._chEnv;
	this->_av = copy._av;
	this->_cgiPath = copy._cgiPath;
	this->_cgiPid = copy._cgiPid;
	this->_exitStatus = copy._exitStatus;
}

Cgi &Cgi::operator=(const Cgi &copy)
{
	this->_env = copy._env;
	this->_chEnv = copy._chEnv;
	this->_av = copy._av;
	this->_cgiPath = copy._cgiPath;
	this->_cgiPid = copy._cgiPid;
	this->_exitStatus = copy._exitStatus;
	return (*this);
}

Cgi::~Cgi()
{
	clear();
}

void Cgi::setCgiPath(const std::string &_cgiPath)
{
	this->_cgiPath = _cgiPath;
}

const pid_t &Cgi::getCgiPid() const
{
	return (this->_cgiPid);
}

const std::string &Cgi::getCgiPath() const
{
	return (this->_cgiPath);
}

std::string Cgi::getPathInfo(std::string &path, std::vector<std::string> extensions)
{
	std::string tmp;
	size_t start, end;	
	for (std::vector<std::string>::iterator itNext = extensions.begin(); itNext != extensions.end(); itNext++)
	{
		start = path.find(*itNext);
		if (start != std::string::npos)
			break;
	}
	if (start == std::string::npos)
		return "";
	if (start + 3 >= path.size())
		return "";
	tmp = path.substr(start + 3, path.size());
	if (tmp.empty() || tmp[0] != '/')
		return "";
	end = tmp.find("?");
	return (end == std::string::npos ? tmp : tmp.substr(0, end));
}

void Cgi::setContentLength(int length)
{
	std::stringstream out;
	out << length;
	_env["CONTENT_LENGTH"] = out.str();
}

void Cgi::setContentType(const std::string &contentType)
{
	_env["CONTENT_TYPE"] = contentType;
}

void Cgi::setDefaultEnvValues(Request &req, std::string cgiExec)
{
	_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	_env["SCRIPT_NAME"] = cgiExec;
	_env["SCRIPT_FILENAME"] = _cgiPath;
	_env["PATH_INFO"] = _cgiPath;
	_env["PATH_TRANSLATED"] = _cgiPath;
	_env["REQUEST_URI"] = _cgiPath;
	_env["SERVER_NAME"] = req.getHeader("host");
	_env["SERVER_PORT"] = "8008";
	_env["REQUEST_METHOD"] = req.getMethodStr();
	_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	_env["REDIRECT_STATUS"] = "200";
	_env["SERVER_SOFTWARE"] = "CHEETAHS";
}

void Cgi::setRequestHeaders(Request &req)
{
	std::map<std::string, std::string> requestHeaders = req.getHeaders();
	for (std::map<std::string, std::string>::iterator it = requestHeaders.begin(); it != requestHeaders.end(); ++it)
	{
		std::string name = it->first;
		std::transform(name.begin(), name.end(), name.begin(), ::toupper);
		std::string key = "HTTP_" + name;
		_env[key] = it->second;
	}
}

void Cgi::createAv(const std::string &cgiExec)
{
	_av = new char *[3];
	_av[0] = strdup(cgiExec.c_str());
	_av[1] = strdup(_cgiPath.c_str());
	_av[2] = NULL;
}

void Cgi::createChEnv()
{
	_chEnv = new char *[_env.size() + 1];
	int i = 0;
	for (std::map<std::string, std::string>::const_iterator it = _env.begin(); it != _env.end(); ++it)
	{
		std::string tmp = it->first + "=" + it->second;
		_chEnv[i] = strdup(tmp.c_str());
		i++;
	}
	_chEnv[i] = NULL;
}

void Cgi::initEnv(Request &req, const std::vector<Location>::iterator itLocation)
{
	int poz;
	std::string extension;
	std::string extPath;	
	extension = this->_cgiPath.substr(this->_cgiPath.find("."));
	std::map<std::string, std::string>::iterator itPath = itLocation->extPath.find(extension);
	if (itPath == itLocation->extPath.end())
		return;
	extPath = itLocation->extPath[extension];	
	this->_env["AUTH_TYPE"] = "Basic";
	this->_env["CONTENT_LENGTH"] = req.getHeader("content-length");
	this->_env["CONTENT_TYPE"] = req.getHeader("content-type");
	this->_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	poz = findStart(this->_cgiPath, "cgi/");
	this->_env["SCRIPT_NAME"] = this->_cgiPath;
	this->_env["SCRIPT_FILENAME"] = ((poz < 0 || (size_t)(poz + 8) > this->_cgiPath.size()) ? "" : this->_cgiPath.substr(poz + 8, this->_cgiPath.size()));
	this->_env["PATH_INFO"] = getPathInfo(req.getPath(), itLocation->getCgiExtension());
	this->_env["PATH_TRANSLATED"] = itLocation->getRootLocation() + (this->_env["PATH_INFO"] == "" ? "/" : this->_env["PATH_INFO"]);
	this->_env["QUERY_STRING"] = decode(req.getQuery());
	this->_env["REMOTE_ADDR"] = req.getHeader("host");
	poz = findStart(req.getHeader("host"), ":");
	this->_env["SERVER_NAME"] = (poz > 0 ? req.getHeader("host").substr(0, poz) : "");
	this->_env["SERVER_PORT"] = (poz > 0 ? req.getHeader("host").substr(poz + 1, req.getHeader("host").size()) : "");
	this->_env["REQUEST_METHOD"] = req.getMethodStr();
	this->_env["HTTP_COOKIE"] = req.getHeader("cookie");
	this->_env["DOCUMENT_ROOT"] = itLocation->getRootLocation();
	this->_env["REQUEST_URI"] = req.getPath() + req.getQuery();
	this->_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	this->_env["REDIRECT_STATUS"] = "200";
	this->_env["SERVER_SOFTWARE"] = "miau";	
	createChEnv();
	createAv(extPath);
}

void Cgi::initEnvCgi(Request &req, const std::vector<Location>::iterator itLocation)
{
	std::string cgiExec = ("cgi/" + itLocation->getCgiPath()[0]).c_str();
	char *cwd = getcwd(NULL, 0);
	if (_cgiPath[0] != '/')
	{
		std::string tmp(cwd);
		tmp.append("/");
		if (!_cgiPath.empty())
			_cgiPath.insert(0, tmp);
	}	
	if (req.getHttpMethod() == POST)
	{
		setContentLength(req.getBody().length());
		setContentType(req.getHeader("content-type"));
	}	
	setDefaultEnvValues(req, cgiExec);
	setRequestHeaders(req);	
	createChEnv();
	createAv(cgiExec);	
	delete[] cwd;
}

void Cgi::execute(short &errorCode)
{
	if (!this->_av[0] || !this->_av[1])
	{
		PrintApp::printEvent(RED_BOLD, SUCCESS, "CGI execute failed: Invalid arguments.");
		errorCode = 500;
		return;
	}
	if (pipe(pipeIn) < 0 || pipe(pipeOut) < 0)
	{
		PrintApp::printEvent(RED_BOLD, SUCCESS, "Failed to create pipes for CGI: %s.", strerror(errno));
		close(pipeIn[0]);
		close(pipeIn[1]);
		close(pipeOut[0]);
		close(pipeOut[1]);
		errorCode = 500;
		return;
	}
	this->_cgiPid = fork();
	if (this->_cgiPid == 0)
	{
		dup2(pipeIn[0], STDIN_FILENO);
		dup2(pipeOut[1], STDOUT_FILENO);
		close(pipeIn[0]);
		close(pipeIn[1]);
		close(pipeOut[0]);
		close(pipeOut[1]);
		this->_exitStatus = execve(this->_av[0], this->_av, this->_chEnv);
		fprintf(stderr, "execve failed: %s\n", strerror(errno));
		exit(this->_exitStatus);
	}
	else if (this->_cgiPid < 0)
	{
		PrintApp::printEvent(RED_BOLD, SUCCESS, "Fork failed: %s.", strerror(errno));
		close(pipeIn[0]);
		close(pipeIn[1]);
		close(pipeOut[0]);
		close(pipeOut[1]);
		errorCode = 500;
		return;
	}
	close(pipeIn[0]);
	close(pipeOut[1]);
}

bool Cgi::isFileUpload()
{
	std::map<std::string, std::string>::iterator it = this->_env.find("CONTENT_TYPE");
	if (it != this->_env.end())
		return it->second.find("multipart/form-data") != std::string::npos;
	return false;
}

int Cgi::findStart(const std::string path, const std::string delim)
{
	if (path.empty())
		return (-1);
	return path.find(delim);
}

std::string Cgi::decode(std::string &path)
{
	std::string decodedPath = path;
	size_t token = decodedPath.find("%");
	while (token != std::string::npos)
	{
		if (decodedPath.length() < token + 2)
			break;
		char decimal = fromHexToDec(decodedPath.substr(token + 1, 2));
		decodedPath.replace(token, 3, toString(decimal));
		token = decodedPath.find("%");
	}
	return decodedPath;
}

unsigned int Cgi::fromHexToDec(const std::string &nb)
{
	unsigned int result = 0;
	std::size_t i = 0;
	while (i < nb.length())
	{
		char c = nb[i];
		if (!std::isxdigit(c))
			return 0;
		result *= 16;
		if (c >= '0' && c <= '9')
			result += c - '0';
		else if (c >= 'A' && c <= 'F')
			result += c - 'A' + 10;
		else if (c >= 'a' && c <= 'f')
			result += c - 'a' + 10;
		++i;
	}
	return result;
}

void Cgi::clear()
{
	_cgiPid = -1;
	_exitStatus = 0;
	_cgiPath.clear();
	_cgiPath = "";
	if (_chEnv)
	{
		for (int i = 0; _chEnv[i] != NULL; ++i)
			delete[] _chEnv[i];
		delete[] _chEnv;
		_chEnv = NULL;
	}
	if (_av)
	{
		delete[] _av[0];
		delete[] _av[1];
		delete[] _av;
		_av = NULL;
	}
	_env.clear();
}
