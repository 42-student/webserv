#include "../inc/Parser.hpp"

Parser::Parser() : _servNbs(0) {}

Parser::~Parser() {}

int Parser::parseServerConfig(const std::string& filePath)
{
	ConfigFile configFile;
	int fileType = configFile.getTypePath(filePath);
	if (fileType == INVALID_TYPE)
		throw Error("Invalid file type detected.");
	int isReadable = configFile.checkAccessFile(filePath, R_OK);
	if (isReadable == -1)
		throw Error("File is not accessible or cannot be read.");
	std::string fileContent = configFile.readFile(filePath);
	if (fileContent.empty())
		throw Error("Empty file encountered.");
	extractServerBlocks(fileContent);
	if (_servConf.size() != _servNbs)
		throw Error("Inconsistent size detected.");
	for (size_t i = 0; i < _servNbs; ++i)
	{
		Server server = parsServConf(_servConf[i]);
		_servList.push_back(server);
	}
	if (_servNbs > 1)
		checkDuplicateServers();
	return 0;
}

std::vector<Server> Parser::getServers() const
{
	return _servList;
}

std::string Parser::trimWhitespace(const std::string& str) const
{
	std::string result = str;
	size_t start = 0;
	while (start < result.size() && std::isspace(result[start]))
		++start;
	result.erase(0, start);
	size_t end = result.size();
	while (end > 0 && std::isspace(result[end - 1]))
		--end;
	result.erase(end);
	return result;
}

std::string Parser::removeComments(const std::string& str) const
{
	std::string result = str;
	size_t hashtagIndex = result.find('#');
	while (hashtagIndex != std::string::npos)
	{
		size_t newlineIndex = result.find('\n', hashtagIndex);
		if (newlineIndex == std::string::npos)
			newlineIndex = result.size();
		result.erase(hashtagIndex, newlineIndex - hashtagIndex);
		hashtagIndex = result.find('#');
	}
	return result;
}

void Parser::extractServerBlocks(const std::string& content)
{
	std::string cleanedContent = removeComments(content);
	cleanedContent = trimWhitespace(cleanedContent);
	if (cleanedContent.find("server") == std::string::npos)
		throw Error("Server not found");
	size_t start = 0;
	size_t end = 0;
	while (start < cleanedContent.size())
	{
		start = findServerStart(start, cleanedContent);
		end = findServerEnd(start, cleanedContent);
		if (start == end)
			throw Error("Scope problem encountered. Start and end positions are the same.");
		_servConf.push_back(cleanedContent.substr(start, end - start + 1));
		++_servNbs;
		start = end + 1;
	}
}

size_t Parser::findServerStart(size_t pos, const std::string& content) const
{
	size_t i = pos;
	while (i < content.size())
	{
		if (content[i] == 's')
		{
			if (content.compare(i, 6, "server") == 0)
			{
				i += 6;
				while (i < content.size() && std::isspace(content[i]))
					++i;
				if (i < content.size() && content[i] == '{')
					return i;
				throw Error("Unexpected character found outside of server scope '{}'.");
			}
			break;
		}
		if (!std::isspace(content[i]))
			throw Error("Encountered an unexpected character outside of server scope '{}'.");
		++i;
	}
	return pos;
}

size_t Parser::findServerEnd(size_t pos, const std::string& content) const
{
	size_t scope = 0;
	size_t i = pos + 1;
	while (i < content.size())
	{
		if (content[i] == '{')
			++scope;
		else if (content[i] == '}')
		{
			if (scope == 0)
				return i;
			--scope;
		}
		++i;
	}
	return pos;
}

Server Parser::parsServConf(const std::string& config)
{
	Server server;
	std::vector<std::string> params = tokenize(config + " ", " \n\t");
	validateParameterCount(params);
	std::vector<std::string> errorCodes;
	int locationFlag = 1;
	bool isAutoindexSet = false;
	bool isMaxSizeSet = false;
	for (size_t i = 0; i < params.size(); ++i)
		processDirective(params[i], i, params, server, errorCodes, locationFlag, isAutoindexSet, isMaxSizeSet);
	setDefaultServerValues(server);
	validateServer(server);
	server.setErrorPages(errorCodes);
	return server;
}

std::vector<std::string> Parser::tokenize(const std::string& str, const std::string& delimiters) const
{
	std::vector<std::string> tokens;
	size_t start = 0;
	size_t end = str.find_first_of(delimiters, start);
	while (end != std::string::npos)
	{
		std::string token = str.substr(start, end - start);
		if (!token.empty())
			tokens.push_back(token);
		start = str.find_first_not_of(delimiters, end);
		if (start == std::string::npos)
			break;
		end = str.find_first_of(delimiters, start);
	}
	return tokens;
}

void Parser::validateParameterCount(const std::vector<std::string>& params) const
{
	if (params.size() < 3)
		throw Error("Server validation failed. Insufficient number of parameters.");
}

void Parser::processDirective(const std::string& directive, size_t& i, const std::vector<std::string>& params, Server& server,
								std::vector<std::string>& errorCodes, int& locationFlag, bool& isAutoindexSet, bool& isMaxSizeSet)
{
	if (directive == "listen" && (i + 1) < params.size() && locationFlag)
	{
		checkDuplicatePort(server);
		server.setPort(params[++i]);
	}
	else if (directive == "location" && (i + 1) < params.size())
	{
		validateScopeChar(params[++i]);
		std::string path = params[i];
		std::vector<std::string> codes;
		if (params[++i] != "{")
			throw Error("Unexpected character encountered in server scope '{}'.");
		while (++i < params.size() && params[i] != "}")
			codes.push_back(params[i]);
		server.setLocation(path, codes);
		validateClosingBracket(params, i);
		locationFlag = 0;
	}
	else if (directive == "host" && (i + 1) < params.size() && locationFlag)
	{
		checkDuplicateHost(server);
		server.setHost(params[++i]);
	}
	else if (directive == "root" && (i + 1) < params.size() && locationFlag)
	{
		checkDuplicateRoot(server);
		server.setRoot(params[++i]);
	}
	else if (directive == "error_page" && (i + 1) < params.size() && locationFlag)
	{
		while (++i < params.size())
		{
			errorCodes.push_back(params[i]);
			if (params[i].find(';') != std::string::npos)
				break;
			if (i + 1 >= params.size())
				throw Error("Invalid syntax: Unexpected character outside server scope. Found: '" + params[i] + "'.");
		}
	}
	else if (directive == "client_max_body_size" && (i + 1) < params.size() && locationFlag)
	{
		checkDuplicateMaxBodySize(isMaxSizeSet);
		server.setClientMaxBodySize(params[++i]);
		isMaxSizeSet = true;
	}
	else if (directive == "server_name" && (i + 1) < params.size() && locationFlag)
	{
		checkDuplicateServerName(server);
		server.setServerName(params[++i]);
	}
	else if (directive == "index" && (i + 1) < params.size() && locationFlag)
	{
		checkDuplicateIndex(server);
		server.setIndex(params[++i]);
	}
	else if (directive == "redirect" && (i + 1) < params.size() && locationFlag)
	{
		checkDuplicateRedirect(server);
		server.setRedirect(params[++i]);
	}
	else if (directive == "autoindex" && (i + 1) < params.size() && locationFlag)
	{
		checkDuplicateAutoindex(isAutoindexSet);
		server.setAutoindex(params[++i]);
		isAutoindexSet = true;
	}
	else if (directive != "}" && directive != "{")
	{
		if (!locationFlag)
			throw Error("Unexpected parameters found after the 'location' directive.");
		throw Error("Unsupported directive encountered.");
	}
}

void Parser::checkDuplicatePort(Server& server)
{
	if (server.getPort())
		throw Error("Duplicate port detected. Port is already assigned.");
}

void Parser::checkDuplicateHost(Server& server)
{
	if (server.getHost())
		throw Error("Duplicate host detected. Host is already assigned.");
}

void Parser::checkDuplicateRoot(Server& server)
{
	if (!server.getRoot().empty())
		throw Error("Duplicate root directory detected. Root directory is already assigned.");
}

void Parser::checkDuplicateMaxBodySize(bool isSet) const
{
	if (isSet)
		throw Error("Invalid syntax: Duplicate declaration of 'client_max_body_size'.");
}

void Parser::checkDuplicateServerName(Server& server)
{
	if (!server.getServerName().empty())
		throw Error("Invalid syntax: Duplicate declaration of 'server_name'.");
}

void Parser::checkDuplicateIndex(Server& server)
{
	if (!server.getIndex().empty())
		throw Error("Invalid syntax: Duplicate declaration of 'index'.");
}

void Parser::checkDuplicateRedirect(Server& server)
{
	if (!server.getRedirect().empty())
		throw Error("Invalid syntax: Duplicate declaration of 'redirect'.");
}

void Parser::checkDuplicateAutoindex(bool isSet) const
{
	if (isSet)
		throw Error("Invalid syntax: Duplicate declaration of 'autoindex'.");
}

void Parser::validateScopeChar(const std::string& param) const
{
	if (param == "{" || param == "}")
		throw Error("Invalid character found in server scope '{}'.");
}

void Parser::validateClosingBracket(const std::vector<std::string>& params, size_t i) const
{
	if (i < params.size() && params[i] != "}")
		throw Error("Invalid character in server scope. Expected '}', but found: '" + params[i] + "'.");
}

void Parser::setDefaultServerValues(Server& server)
{
	if (server.getRoot().empty())
		server.setRoot("/;");
	if (!server.getHost())
		server.setHost("localhost;");
	if (server.getIndex().empty())
		server.setIndex("index.html;");
}

void Parser::validateServer(Server& server)
{
	if (ConfigFile::CheckFile(server.getRoot(), server.getIndex()))
		throw Error("The index file specified in the config file was not found or is unreadable.");
	if (server.checkLocation())
		throw Error("Duplicate location found in the server configuration.");
	if (!server.getPort())
		throw Error("Port number is missing in the server configuration.");
	if (!server.isValidErrorPages())
		throw Error("Incorrect error page path or invalid number of error pages specified.");
}

bool Parser::isDuplicateServer(Server& curr, Server& next)
{
	return (curr.getPort() == next.getPort() && curr.getHost() == next.getHost() && curr.getServerName() == next.getServerName());
}

void Parser::checkDuplicateServers()
{
	for (size_t i = 0; i < _servList.size() - 1; ++i)
	{
		for (size_t j = i + 1; j < _servList.size(); ++j)
		{
			if (isDuplicateServer(_servList[i], _servList[j]))
				throw Error("Duplicate server configuration detected. Servers must have unique combinations of port, host, and server name.");
		}
	}
}
