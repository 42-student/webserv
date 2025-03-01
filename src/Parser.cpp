#include "../inc/Parser.hpp"

Parser::Parser()
{
	this->_nbOfServ = 0;
}

Parser::~Parser() {}

int Parser::parsServConf(const std::string &filePath)
{
	ConfigFile configFile;
	int fileType = configFile.getTypePath(filePath);	
	if (fileType == INVALID_TYPE)
		throw Error("Invalid file type detected.");	
	int isFileReadable = configFile.checkAccessFile(filePath, R_OK);	
	if (isFileReadable == -1)
		throw Error("File is not accessible or cannot be read.");	
	std::string fileContent = configFile.readFile(filePath);	
	if (fileContent.empty())
		throw Error("Empty file encountered.");	
	extractServConf(fileContent);	
	if (this->_servConf.size() != this->_nbOfServ)
		throw Error("Inconsistent size detected.");	
	for (size_t i = 0; i < this->_nbOfServ; i++)
	{
		Server server;
		createServFromConf(this->_servConf[i], server);
		this->_servList.push_back(server);
	}	
	if (this->_nbOfServ > 1)
		checkDupServConf();	
	return (0);
}

const std::string Parser::extractServConf(std::string &fileContent)
{
	std::string contentWithoutComments = this->removeComments(fileContent);
	std::string contentWithoutSpaces = this->removeSpaces(contentWithoutComments);
	this->splitServers(contentWithoutSpaces);	
	return contentWithoutSpaces;
}

const std::string Parser::removeComments(std::string &string)
{
	int hashtagIndex = string.find('#');
	while (hashtagIndex >= 0)
	{
		int newLineIndex = string.find('\n', hashtagIndex);
		string.erase(hashtagIndex, newLineIndex - hashtagIndex);
		hashtagIndex = string.find('#');
	}
	return string;
}

const std::string Parser::removeSpaces(std::string &string)
{
	size_t start = 0;
	while (start < string.size() && std::isspace(string[start]))
		start++;	
	string.erase(0, start);
	size_t end = string.size() - 1;	
	while (end > 0 && std::isspace(string[end]))
		end--;	
	string.erase(end + 1);
	return string;
}

void Parser::splitServers(std::string &content)
{
	size_t start = 0;
	size_t end = 1;	
	if (content.find("server", 0) == std::string::npos)
		throw Error("Server not found");
	while (start != end && start < content.length())
	{
		start = findStartServ(start, content);
		end = findEndServ(start, content);
		if (start == end)
			throw Error("Scope problem encountered. Start and end positions are the same.");
		this->_servConf.push_back(content.substr(start, end - start + 1));
		this->_nbOfServ++;
		start = end + 1;
	}
}

size_t Parser::findStartServ(size_t start, std::string &content)
{
	size_t i;	
	for (i = start; content[i]; i++)
	{
		if (content[i] == 's')
			break;
		if (!isspace(content[i]))
			throw Error("Encountered an unexpected character outside of server scope '{}'.");
	}
	if (!content[i])
		return (start);
	if (content.compare(i, 6, "server") != 0)
		throw Error("Unexpected character found outside of server scope '{}'.");
	i += 6;
	while (content[i] && isspace(content[i]))
		i++;
	if (content[i] == '{')
		return (i);
	throw Error("Unexpected character found outside of server scope '{}'.");
}

size_t Parser::findEndServ(size_t start, std::string &content)
{
	size_t i;
	size_t scope;	
	scope = 0;
	for (i = start + 1; content[i]; i++)
	{
		if (content[i] == '{')
			scope++;
		if (content[i] == '}')
		{
			if (!scope)
				return (i);
			scope--;
		}
	}
	return (start);
}

void Parser::createServFromConf(std::string &configString, Server &server)
{
	std::vector<std::string> parameterList;
	std::vector<std::string> errorCode;
	int locationFlag = 1;
	bool isAutoindexEnable = false;
	bool isMaxSizeSet = false;	
	parameterList = splitParam(configString += ' ', std::string(" \n\t"));
	valServParamSize(parameterList);	
	for (size_t i = 0; i < parameterList.size(); i++)
	{
		if (parameterList[i] == "listen" && (i + 1) < parameterList.size() && locationFlag)
		{
			valDupPort(server);
			server.setPort(parameterList[++i]);
		}
		else if (parameterList[i] == "location" && (i + 1) < parameterList.size())
		{
			valServScopeChar(parameterList[++i]);
			std::string path = parameterList[i];
			std::vector<std::string> codes = parsLocCodes(parameterList, i);
			server.setLocation(path, codes);
			valClosingBracket(parameterList, i);
			locationFlag = 0;
		}
		else if (parameterList[i] == "host" && (i + 1) < parameterList.size() && locationFlag)
		{
			valDupHost(server);
			server.setHost(parameterList[++i]);
		}
		else if (parameterList[i] == "root" && (i + 1) < parameterList.size() && locationFlag)
		{
			valDupRoot(server);
			server.setRoot(parameterList[++i]);
		}
		else if (parameterList[i] == "error_page" && (i + 1) < parameterList.size() && locationFlag)
		{
			processErrorCodes(parameterList, i, errorCode);
		}
		else if (parameterList[i] == "client_max_body_size" && (i + 1) < parameterList.size() && locationFlag)
		{
			valDupMaxBodySize(isMaxSizeSet);
			server.setClientMaxBodySize(parameterList[++i]);
			isMaxSizeSet = true;
		}
		else if (parameterList[i] == "server_name" && (i + 1) < parameterList.size() && locationFlag)
		{
			valDupServName(server);
			server.setServerName(parameterList[++i]);
		}
		else if (parameterList[i] == "index" && (i + 1) < parameterList.size() && locationFlag)
		{
			valDupIndex(server);
			server.setIndex(parameterList[++i]);
		}
		else if (parameterList[i] == "redirect" && (i + 1) < parameterList.size() && locationFlag)
		{
			valDupRedirect(server);
			server.setRedirect(parameterList[++i]);
		}
		else if (parameterList[i] == "autoindex" && (i + 1) < parameterList.size() && locationFlag)
		{
			valDupAutoindex(isAutoindexEnable);
			server.setAutoindex(parameterList[++i]);
			isAutoindexEnable = true;
		}
		else if (parameterList[i] != "}" && parameterList[i] != "{")
		{
			if (!locationFlag)
				throw Error("Unexpected parameters found after the 'location' directive.");
			throw Error("Unsupported directive encountered.");
		}
	}
	setDefaultServVal(server);
	servValidations(server);
	server.setErrorPages(errorCode);
}

std::vector<std::string> Parser::splitParam(std::string inputStr, std::string delimeter)
{
	std::vector<std::string> parameterList;
	std::string::size_type startPosition = 0;
	std::string::size_type endPosition = inputStr.find_first_of(delimeter, startPosition);	
	while (endPosition != std::string::npos)
	{
		std::string currentSegment = inputStr.substr(startPosition, endPosition - startPosition);
		parameterList.push_back(currentSegment);
		startPosition = inputStr.find_first_not_of(delimeter, endPosition);
		if (startPosition == std::string::npos)
			break;
		endPosition = inputStr.find_first_of(delimeter, startPosition);
	}
	return (parameterList);
}

void Parser::valServParamSize(const std::vector<std::string> &parameters)
{
	if (parameters.size() < 3)
		throw Error("Server validation failed. Insufficient number of parameters.");
}

void Parser::valDupPort(Server &server)
{
	if (server.getPort())
		throw Error("Duplicate port detected. Port is already assigned.");
}

void Parser::valServScopeChar(const std::string &parameter)
{
	if (parameter == "{" || parameter == "}")
		throw Error("Invalid character found in server scope '{}'.");
}

std::vector<std::string> Parser::parsLocCodes(const std::vector<std::string> &parameters, size_t &i)
{
	std::vector<std::string> codes;
	if (parameters[++i] != "{")
		throw Error("Unexpected character encountered in server scope '{}'.");
	i++;
	while (i < parameters.size() && parameters[i] != "}")
		codes.push_back(parameters[i++]);
	return codes;
}

void Parser::valClosingBracket(const std::vector<std::string> &parameter, size_t &i)
{
	if (i < parameter.size() && parameter[i] != "}")
		throw Error("Invalid character in server scope. Expected '}', but found: '" + parameter[i] + "'.");
}

void Parser::valDupHost(Server &server)
{
	if (server.getHost())
		throw Error("Duplicate host detected. Host is already assigned.");
}

void Parser::valDupRoot(Server &server)
{
	if (!server.getRoot().empty())
		throw Error("Duplicate root directory detected. Root directory is already assigned.");
}

void Parser::processErrorCodes(const std::vector<std::string> &parameters, size_t &i, std::vector<std::string> &errorCodes)
{
	while (++i < parameters.size())
	{
		errorCodes.push_back(parameters[i]);
		if (parameters[i].find(';') != std::string::npos)
			break;
		if (i + 1 >= parameters.size())
			throw Error("Invalid syntax: Unexpected character outside server scope. Found: '" + parameters[i] + "'.");
	}
}

void Parser::valDupMaxBodySize(bool isMaxSizeSet)
{
	if (isMaxSizeSet)
		throw Error("Invalid syntax: Duplicate declaration of 'client_max_body_size'.");
}

void Parser::valDupServName(Server &server)
{
	if (!server.getServerName().empty())
		throw Error("Invalid syntax: Duplicate declaration of 'server_name'.");
}

void Parser::valDupIndex(Server &server)
{
	if (!server.getIndex().empty())
		throw Error("Invalid syntax: Duplicate declaration of 'index'.");
}

void Parser::valDupRedirect(Server &server)
{
	if (!server.getRedirect().empty())
		throw Error("Invalid syntax: Duplicate declaration of 'redirect'.");
}

void Parser::valDupAutoindex(bool flag_autoindex)
{
	if (flag_autoindex)
		throw Error("Invalid syntax: Duplicate declaration of 'autoindex'.");
}

void Parser::setDefaultServVal(Server &server)
{
	if (server.getRoot().empty())
		server.setRoot("/;");
	if (!server.getHost())
		server.setHost("localhost;");
	if (server.getIndex().empty())
		server.setIndex("index.html;");
}

void Parser::servValidations(Server &server)
{
	if (ConfigFile::CheckFile(server.getRoot(), server.getIndex()))
		throw Error("The index file specified in the config file was not found or is unreadable.");
	if (server.checkLocation())
		throw Error("Duplicate location found in the server configuration.");
	if (!server.getPort())
		throw Error("Port number is missing in the server configuration");
	if (!server.isValidErrorPages())
		throw Error("Incorrect error page path or invalid number of error pages specified.");
}

bool Parser::checkIfServDup(Server &currentServer, Server &nextServer)
{
	bool isPortDuplicate = (currentServer.getPort() == nextServer.getPort());
	bool isHostDuplicate = (currentServer.getHost() == nextServer.getHost());
	bool isNameDuplicate = (currentServer.getServerName() == nextServer.getServerName());	
	return (isPortDuplicate && isHostDuplicate && isNameDuplicate);
}

void Parser::checkDupServConf()
{
	std::vector<Server>::iterator currentServer;
	std::vector<Server>::iterator nextServer;	
	for (currentServer = _servList.begin(); currentServer != _servList.end() - 1; currentServer++)
	{
		nextServer = currentServer;
		++nextServer;
		while (nextServer != _servList.end())
		{
			if (checkIfServDup(*currentServer, *nextServer))
				throw Error("Duplicate server configuration detected. Servers must have unique combinations of port, host, and server name.");	
			++nextServer;
		}
	}
}

std::vector<Server> Parser::getServers()
{
	return (this->_servList);
}
