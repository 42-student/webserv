#include "../inc/Location.hpp"

Location::Location()
{
	this->_locPath = "";
	this->_locRoot = "";
	this->_locAutoIndex = false;
	this->_locIndex = "";
	this->_returns = "";
	this->_alias = "";
	this->_maxBodySize = MAX_CONTENT_LENGTH;
	this->_locRedirect = "";
	this->_methods.reserve(3);
	this->_methods.push_back(1);
	this->_methods.push_back(0);
	this->_methods.push_back(0);
}

Location::Location(const Location &copy)
{
	this->_locPath = copy._locPath;
	this->_locRoot = copy._locRoot;
	this->_locAutoIndex = copy._locAutoIndex;
	this->_locIndex = copy._locIndex;
	this->_cgiPath = copy._cgiPath;
	this->_cgiExt = copy._cgiExt;
	this->_returns = copy._returns;
	this->_alias = copy._alias;
	this->_methods = copy._methods;
	this->extPath = copy.extPath;
	this->_maxBodySize = copy._maxBodySize;
	this->_locRedirect = copy._locRedirect;
}

Location &Location::operator=(const Location &copy)
{
	this->_locPath = copy._locPath;
	this->_locRoot = copy._locRoot;
	this->_locAutoIndex = copy._locAutoIndex;
	this->_locIndex = copy._locIndex;
	this->_cgiPath = copy._cgiPath;
	this->_cgiExt = copy._cgiExt;
	this->_returns = copy._returns;
	this->_alias = copy._alias;
	this->_methods = copy._methods;
	this->extPath = copy.extPath;
	this->_maxBodySize = copy._maxBodySize;
	this->_locRedirect = copy._locRedirect;
	return (*this);
}

Location::~Location() {}

void Location::setPath(std::string param)
{
	this->_locPath = param;
}

void Location::setRootLocation(std::string param)
{
	if (ConfigFile::getTypePath(param) == 2)
		this->_locRoot = param;
	else
		throw Error("Invalid root location. Please provide a valid path for the root location.");
}

void Location::setMethods(std::vector<std::string> methods)
{
	this->_methods[0] = 0;
	this->_methods[1] = 0;
	this->_methods[2] = 0;
	for (size_t i = 0; i < methods.size(); i++) {
		if (methods[i] == "GET")
			this->_methods[0] = 1;
		else if (methods[i] == "POST")
			this->_methods[1] = 1;
		else if (methods[i] == "DELETE")
			this->_methods[2] = 1;
		else
			throw Error("Unsupported HTTP method: " + methods[i] + ". Only GET, POST, and DELETE methods are supported.");
	}
}

void Location::setAutoindex(std::string param)
{
	if (param == "on" || param == "off")
		this->_locAutoIndex = (param == "on");
	else
		throw Error("Incorrect autoindex value. Please specify 'on' or 'off' for the autoindex parameter.");
}

void Location::setIndexLocation(std::string param)
{
	this->_locIndex = param;
}

void Location::setReturn(std::string param)
{
	this->_returns = param;
}

void Location::setAlias(std::string param)
{
	this->_alias = param;
}

void Location::setCgiPath(std::vector<std::string> path)
{
	this->_cgiPath = path;
}

void Location::setCgiExtension(std::vector<std::string> extension)
{
	this->_cgiExt = extension;
}

void Location::setMaxBodySize(std::string param)
{
	unsigned long bodySize = 0;
	for (size_t i = 0; i < param.length(); i++) {
		if (param[i] < '0' || param[i] > '9')
			throw Error("Incorrect syntax: client_max_body_size. Only numerical digits are allowed.");
	}
	if (!atoi(param.c_str()))
		throw Error("Incorrect syntax: client_max_body_size. The parameter should be a positive integer.");
	bodySize = atoi(param.c_str());
	this->_maxBodySize = bodySize;
}

void Location::setMaxBodySize(unsigned long param)
{
	this->_maxBodySize = param;
}

void Location::setRedirect(std::string param)
{
	this->_locRedirect = param;
}

const std::string &Location::getPath() const
{
	return (this->_locPath);
}

const std::string &Location::getRootLocation() const
{
	return (this->_locRoot);
}

const std::vector<short> &Location::getMethods() const
{
	return (this->_methods);
}

const bool &Location::getAutoindex() const
{
	return (this->_locAutoIndex);
}

const std::string &Location::getIndexLocation() const
{
	return (this->_locIndex);
}

const std::string &Location::getReturn() const
{
	return (this->_returns);
}

const std::string &Location::getAlias() const
{
	return (this->_alias);
}

const std::vector<std::string> &Location::getCgiPath() const
{
	return (this->_cgiPath);
}

const std::vector<std::string> &Location::getCgiExtension() const
{
	return (this->_cgiExt);
}

const unsigned long &Location::getMaxBodySize() const
{
	return (this->_maxBodySize);
}

const std::string &Location::getRedirect() const
{
	return (this->_locRedirect);
}

const std::map<std::string, std::string> &Location::getExtensionPath() const
{
	return (this->extPath);
}
