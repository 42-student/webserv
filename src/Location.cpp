#include "../inc/Location.hpp"

Location::Location() : _locPath(""), _locRoot(""), _methods(), _locAutoIndex(false), _locIndex(""), _returns(""),
					_alias(""), _cgiPath(), _cgiExt(), _maxBodySize(MAX_CONTENT_LENGTH), _locRedirect(""), extPath()
{
	_methods.resize(3);
	_methods[0] = 1;
	_methods[1] = 0;
	_methods[2] = 0;
}

Location::Location(const Location& src)
    : _locPath(src._locPath), _locRoot(src._locRoot), _methods(src._methods), _locAutoIndex(src._locAutoIndex), _locIndex(src._locIndex), _returns(src._returns),
	  _alias(src._alias), _cgiPath(src._cgiPath), _cgiExt(src._cgiExt), _maxBodySize(src._maxBodySize), _locRedirect(src._locRedirect), extPath(src.extPath) {}

Location& Location::operator=(const Location& src)
{
	if (this != &src)
	{
		_locPath = src._locPath;
		_locRoot = src._locRoot;
		_methods = src._methods;
		_locAutoIndex = src._locAutoIndex;
		_locIndex = src._locIndex;
		_returns = src._returns;
		_alias = src._alias;
		_cgiPath = src._cgiPath;
		_cgiExt = src._cgiExt;
		_maxBodySize = src._maxBodySize;
		_locRedirect = src._locRedirect;
		extPath = src.extPath;
	}
	return *this;
}

Location::~Location() {}

void Location::setPath(std::string param)
{
	_locPath = param;
}

void Location::setRootLocation(std::string param)
{
	if (ConfigFile::getTypePath(param) != 2)
		throw Error("Invalid root location. Please provide a valid directory path.");
	_locRoot = param;
}

void Location::setMethods(std::vector<std::string> methods)
{
	_methods[0] = 0;
	_methods[1] = 0;
	_methods[2] = 0;
	for (size_t i = 0; i < methods.size(); ++i)
	{
		if (methods[i] == "GET")
			_methods[0] = 1;
		else if (methods[i] == "POST")
			_methods[1] = 1;
		else if (methods[i] == "DELETE")
			_methods[2] = 1;
		else
			throw Error("Unsupported HTTP method: " + methods[i] + ". Supported methods: GET, POST, DELETE.");
	}
}

void Location::setAutoindex(std::string param)
{
	if (param == "on")
		_locAutoIndex = true;
	else if (param == "off")
		_locAutoIndex = false;
	else
		throw Error("Invalid autoindex value. Use 'on' or 'off' only.");
}

void Location::setIndexLocation(std::string param)
{
	_locIndex = param;
}

void Location::setReturn(std::string param)
{
	_returns = param;
}

void Location::setAlias(std::string param)
{
	_alias = param;
}

void Location::setCgiPath(std::vector<std::string> path)
{
	_cgiPath = path;
}

void Location::setCgiExtension(std::vector<std::string> extension)
{
	_cgiExt = extension;
}

void Location::setMaxBodySize(std::string param)
{
	std::istringstream stream(param);
	unsigned long size;
	char extra;
	if (!(stream >> size) || (stream >> extra))
		throw Error("Invalid client_max_body_size syntax. Must be a positive integer with no extra characters.");
	if (size == 0)
		throw Error("client_max_body_size must be a positive integer greater than zero.");
	_maxBodySize = size;
}

void Location::setMaxBodySize(unsigned long param)
{
	_maxBodySize = param;
}

void Location::setRedirect(std::string param)
{
	_locRedirect = param;
}

const std::string& Location::getPath() const
{
	return _locPath;
}

const std::string& Location::getRootLocation() const
{
	return _locRoot;
}

const std::vector<short>& Location::getMethods() const
{
	return _methods;
}

const bool& Location::getAutoindex() const
{
	return _locAutoIndex;
}

const std::string& Location::getIndexLocation() const
{
	return _locIndex;
}

const std::string& Location::getReturn() const
{
	return _returns;
}

const std::string& Location::getAlias() const
{
	return _alias;
}

const std::vector<std::string>& Location::getCgiPath() const
{
	return _cgiPath;
}

const std::vector<std::string>& Location::getCgiExtension() const
{
	return _cgiExt;
}

const unsigned long& Location::getMaxBodySize() const
{
	return _maxBodySize;
}

const std::string& Location::getRedirect() const
{
	return _locRedirect;
}

const std::map<std::string, std::string>& Location::getExtensionPath() const
{
	return extPath;
}
