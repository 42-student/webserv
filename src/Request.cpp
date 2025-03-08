#include "../inc/Request.hpp"

Request::Request()
{
	_httpMethod = NONE;
	_parsStatus = STATE_REQUEST_LINE;
	_httpMethodStr[::GET] = "GET";
	_httpMethodStr[::POST] = "POST";
	_httpMethodStr[::DELETE] = "DELETE";
	_reqPath = "";
	_query = "";
	_bodyStr = "";
	_boundary = "";
	_storage = "";
	_heyStorage = "";
	_bodyLength = 0;
	_chunkLen = 0;
	_httpMethodIndex = 1;
	_errorCode = 0;
	_bodyFlag = false;
	_bodyDoneFlag = false;
	_fieldsDoneFlag = false;
	_chunkedFlag = false;
	_multiformFlag = false;
}

Request::~Request() {}

void Request::parseHTTPRequestData(char *data, size_t size)
{
	for (size_t i = 0; i < size; ++i)
	{
		u_int8_t c = data[i];
		processChar(c);
		if (_parsStatus == STATE_PARSING_DONE)
			break;
	}
	if (_parsStatus == STATE_PARSING_DONE)
		_bodyStr.assign(reinterpret_cast<char *>(&_reqBody[0]), _reqBody.size());
}

void Request::processChar(u_int8_t c)
{
	if (_parsStatus < STATE_FIELD_NAME_START)
		processRequestLine(c);
	else if (_parsStatus >= STATE_FIELD_NAME_START && _parsStatus <= STATE_FIELD_VALUE_END)
		processHeaders(c);
	else if (_parsStatus >= STATE_CHUNKED_LENGTH_BEGIN && _parsStatus <= STATE_CHUNKED_END_LF)
		processChunked(c);
	else if (_parsStatus == STATE_MESSAGE_BODY)
		processMessageBody(c);
}

void Request::processRequestLine(u_int8_t c)
{
	if (_parsStatus == STATE_REQUEST_LINE)
	{
		if (c == 'G')
			_httpMethod = GET;
		else if (c == 'P')
		{
			_parsStatus = STATE_REQUEST_LINE_POST_PUT;
			return;
		}
		else if (c == 'D')
			_httpMethod = DELETE;
		else if (c == 'H')
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 501, "Unsupported method <%s>.", "HEAD");
			return;
		}
		else if (c == 'O')
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 501, "Unsupported method <%s>.", "OPTIONS");
			return;
		}
		else if (c == 'U')
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 501, "Unsupported method <%s>.", "UNKNOWN");
			return;
		}
		_parsStatus = STATE_REQUEST_LINE_METHOD;
		_storage += c;
		return;
	}
	else if (_parsStatus == STATE_REQUEST_LINE_POST_PUT)
	{
		if (c == 'O')
			_httpMethod = POST;
		else if (c == 'U')
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 501, "Unsupported method <%s>.", "PUT");
			return;
		}
		else if (c == 'A')
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 501, "Unsupported method <%s>.", "PATCH");
			return;
		}
		_httpMethodIndex++;
		_parsStatus = STATE_REQUEST_LINE_METHOD;
		_storage += c;
		return;
	}
	else if (_parsStatus == STATE_REQUEST_LINE_METHOD)
	{
		if (c == _httpMethodStr[_httpMethod][_httpMethodIndex])
			_httpMethodIndex++;
		_storage += c;
		if ((size_t)_httpMethodIndex == _httpMethodStr[_httpMethod].length())
			_parsStatus = STATE_REQUEST_LINE_FIRST_SPACE;
		return;
	}
	else if (_parsStatus == STATE_REQUEST_LINE_FIRST_SPACE)
	{
		if (c != ' ')
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", c);
			return;
		}
		_parsStatus = STATE_REQUEST_LINE_URI_PATH_SLASH;
		return;
	}
	else if (_parsStatus == STATE_REQUEST_LINE_URI_PATH_SLASH)
	{
		if (c == '/')
		{
			_parsStatus = STATE_REQUEST_LINE_URI_PATH;
			_storage = "/";
		}
		else
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", c);
			return;
		}
		return;
	}
	else if (_parsStatus == STATE_REQUEST_LINE_URI_PATH)
	{
		if (c == ' ')
		{
			_parsStatus = STATE_REQUEST_LINE_VER;
			_reqPath.append(_storage);
			_storage = "";
			return;
		}
		else if (c == '?')
		{
			_parsStatus = STATE_REQUEST_LINE_URI_QUERY;
			_reqPath.append(_storage);
			_storage = "";
			return;
		}
		else if (c == '#')
		{
			_parsStatus = STATE_REQUEST_LINE_URI_FRAGMENT;
			_reqPath.append(_storage);
			_storage = "";
			return;
		}
		else if (!isValidURIChar(c))
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", c);
			return;
		}
		else if (_storage.length() > MAX_URI_LENGTH)
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 414, "URI exceeds maximum length.", c);
			return;
		}
		_storage += c;
		return;
	}
	else if (_parsStatus == STATE_REQUEST_LINE_URI_QUERY)
	{
		if (c == ' ')
		{
			_parsStatus = STATE_REQUEST_LINE_VER;
			_query.append(_storage);
			_storage = "";
			return;
		}
		else if (c == '#')
		{
			_parsStatus = STATE_REQUEST_LINE_URI_FRAGMENT;
			_query.append(_storage);
			_storage = "";
			return;
		}
		else if (!isValidURIChar(c))
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", c);
			return;
		}
		else if (_storage.length() > MAX_URI_LENGTH)
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 414, "URI exceeds maximum length.", c);
			return;
		}
		_storage += c;
		return;
	}
	else if (_parsStatus == STATE_REQUEST_LINE_URI_FRAGMENT)
	{
		if (c == ' ')
		{
			_parsStatus = STATE_REQUEST_LINE_VER;
			_storage = "";
			return;
		}
		else if (!isValidURIChar(c))
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", c);
			return;
		}
		else if (_storage.length() > MAX_URI_LENGTH)
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 414, "URI exceeds maximum length.", c);
			return;
		}
		_storage += c;
		return;
	}
	else if (_parsStatus == STATE_REQUEST_LINE_VER)
	{
		if (isValidUriPosition(_reqPath))
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character in URI.");
			return;
		}
		if (c != 'H')
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", c);
			return;
		}
		_parsStatus = STATE_REQUEST_LINE_HT;
		return;
	}
	else if (_parsStatus == STATE_REQUEST_LINE_HT)
	{
		if (c != 'T')
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", c);
			return;
		}
		_parsStatus = STATE_REQUEST_LINE_HTT;
		return;
	}
	else if (_parsStatus == STATE_REQUEST_LINE_HTT)
	{
		if (c != 'T')
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", c);
			return;
		}
		_parsStatus = STATE_REQUEST_LINE_HTTP;
		return;
	}
	else if (_parsStatus == STATE_REQUEST_LINE_HTTP)
	{
		if (c != 'P')
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", c);
			return;
		}
		_parsStatus = STATE_REQUEST_LINE_HTTP_SLASH;
		return;
	}
	else if (_parsStatus == STATE_REQUEST_LINE_HTTP_SLASH)
	{
		if (c != '/')
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", c);
			return;
		}
		_parsStatus = STATE_REQUEST_LINE_MAJOR;
		return;
	}
	else if (_parsStatus == STATE_REQUEST_LINE_MAJOR)
	{
		if (!isdigit(c))
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", c);
			return;
		}
		_verMaj = c;
		_parsStatus = STATE_REQUEST_LINE_DOT;
		return;
	}
	else if (_parsStatus == STATE_REQUEST_LINE_DOT)
	{
		if (c != '.')
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Expected '.' but found \"%c\".", c);
			return;
		}
		_parsStatus = STATE_REQUEST_LINE_MINOR;
		return;
	}
	else if (_parsStatus == STATE_REQUEST_LINE_MINOR)
	{
		if (!isdigit(c))
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", c);
			return;
		}
		_verMin = c;
		_parsStatus = STATE_REQUEST_LINE_CR;
		return;
	}
	else if (_parsStatus == STATE_REQUEST_LINE_CR)
	{
		if (c != '\r')
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Expected CR but found \"%c\".", c);
			return;
		}
		_parsStatus = STATE_REQUEST_LINE_LF;
		return;
	}
	else if (_parsStatus == STATE_REQUEST_LINE_LF)
	{
		if (c != '\n')
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Expected LF but found \"%c\".", c);
			return;
		}
		_parsStatus = STATE_FIELD_NAME_START;
		_storage = "";
		return;
	}
}

void Request::processHeaders(u_int8_t c)
{
	if (_parsStatus == STATE_FIELD_NAME_START)
	{
		if (c == '\r')
		{
			_parsStatus = STATE_FIELDS_END;
			return;
		}
		else if (isValidTokenChar(c))
		{
			_parsStatus = STATE_FIELD_NAME;
			_storage = "";
        	_storage += c;
        	return;
		}
		else
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character in header: \"%c\".", c);
			return;
		}
	}
	else if (_parsStatus == STATE_FIELDS_END)
	{
		if (c == '\n')
		{
			_storage = "";
			_fieldsDoneFlag = true;
			extractRequestHeaders();
			if (_bodyFlag)
				_parsStatus = _chunkedFlag ? STATE_CHUNKED_LENGTH_BEGIN : STATE_MESSAGE_BODY;
			else
				_parsStatus = STATE_PARSING_DONE;
			return;
		}
		else
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Expected LF at end of headers but found \"%c\".", c);
			return;
		}
	}
	else if (_parsStatus == STATE_FIELD_NAME)
	{
		if (c == ':')
		{
			_heyStorage = _storage;
			_storage = "";
			_parsStatus = STATE_FIELD_VALUE;
			return;
		}
		else if (!isValidTokenChar(c))
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Invalid character in header name: \"%c\".", c);
			return;
		}
		_storage += c;
		return;
	}
	else if (_parsStatus == STATE_FIELD_VALUE)
	{
		if (c == '\r')
		{
			setHeader(_heyStorage, _storage);
			_heyStorage = "";
			_storage = "";
			_parsStatus = STATE_FIELD_VALUE_END;
			return;
		}
		_storage += c;
		return;
	}
	else if (_parsStatus == STATE_FIELD_VALUE_END)
	{
		if (c == '\n')
		{
			_parsStatus = STATE_FIELD_NAME_START;
			return;
		}
		else
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Expected LF after header value but found \"%c\".", c);
			return;
		}
	}
}

void Request::processChunked(u_int8_t c)
{
	if (_parsStatus == STATE_CHUNKED_LENGTH_BEGIN)
	{
		if (!isxdigit(c))
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Invalid chunk length character: \"%c\".", c);
			return;
		}
		_storage = "";
		_storage += c;
		std::istringstream iss(_storage);
		iss >> std::hex >> _chunkLen;
		_parsStatus = (_chunkLen == 0) ? STATE_CHUNKED_LENGTH_CR : STATE_CHUNKED_LENGTH;
		return;
	}
	else if (_parsStatus == STATE_CHUNKED_LENGTH)
	{
		if (isxdigit(c))
		{
			_storage += c;
			std::istringstream iss(_storage);
			iss >> std::hex >> _chunkLen;
		}
		else if (c == '\r')
			_parsStatus = STATE_CHUNKED_LENGTH_LF;
		else
			_parsStatus = STATE_CHUNKED_IGNORE;
		return;
	}
	else if (_parsStatus == STATE_CHUNKED_LENGTH_CR)
	{
		if (c == '\r')
			_parsStatus = STATE_CHUNKED_LENGTH_LF;
		else
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Expected CR in chunked length but found \"%c\".", c);
			return;
		}
		return;
	}
	else if (_parsStatus == STATE_CHUNKED_LENGTH_LF)
	{
		if (c == '\n')
			_parsStatus = (_chunkLen == 0) ? STATE_CHUNKED_END_CR : STATE_CHUNKED_DATA;
		else
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Expected LF in chunked length but found \"%c\".", c);
			return;
		}
		return;
	}
	else if (_parsStatus == STATE_CHUNKED_IGNORE)
	{
		if (c == '\r')
			_parsStatus = STATE_CHUNKED_END_LF;
		return;
	}
	else if (_parsStatus == STATE_CHUNKED_DATA)
	{
		_reqBody.push_back(c);
		if (_chunkLen > 0)
			_chunkLen--;
		if (_chunkLen == 0)
			_parsStatus = STATE_CHUNKED_DATA_CR;
		return;
	}
	else if (_parsStatus == STATE_CHUNKED_DATA_CR)
	{
		if (c == '\r')
			_parsStatus = STATE_CHUNKED_DATA_LF;
		else
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Expected CR after chunk data but found \"%c\".", c);
			return;
		}
		return;
	}
	else if (_parsStatus == STATE_CHUNKED_DATA_LF)
	{
		if (c == '\n')
			_parsStatus = STATE_CHUNKED_LENGTH_BEGIN;
		else
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Expected LF after chunk data but found \"%c\".", c);
			return;
		}
		return;
	}
	else if (_parsStatus == STATE_CHUNKED_END_CR)
	{
		if (c != '\r')
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Expected CR in chunked end but found \"%c\".", c);
			return;
		}
		_parsStatus = STATE_CHUNKED_END_LF;
		return;
	}
	else if (_parsStatus == STATE_CHUNKED_END_LF)
	{
		if (c != '\n')
		{
			PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Expected LF in chunked end but found \"%c\".", c);
			return;
		}
		_bodyDoneFlag = true;
		_parsStatus = STATE_PARSING_DONE;
		return;
	}
}

void Request::processMessageBody(u_int8_t c)
{
	_reqBody.push_back(c);
	if (_reqBody.size() >= _bodyLength)
	{
		_bodyDoneFlag = true;
		_parsStatus = STATE_PARSING_DONE;
	}
}

bool Request::isValidUriPosition(std::string path)
{
	std::string tmp(path);
	char *res = strtok((char *)tmp.c_str(), "/");
	int pos = 0;
	while (res != NULL)
	{
		if (!strcmp(res, ".."))
			pos--;
		else
			pos++;
		if (pos < 0)
			return (true);
		res = strtok(NULL, "/");
	}
	return (false);
}

bool Request::isValidURIChar(uint8_t ch)
{
	if ((ch >= '#' && ch <= ';') || (ch >= '?' && ch <= '[') || (ch >= 'a' && ch <= 'z') ||
		ch == '!' || ch == '=' || ch == ']' || ch == '_' || ch == '~')
		return (true);
	return (false);
}

bool Request::isValidTokenChar(uint8_t ch)
{
	if (ch == '!' || (ch >= '#' && ch <= '\'') || ch == '*' || ch == '+' || ch == '-' || ch == '.' ||
		(ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') || (ch >= '^' && ch <= '`') ||
		(ch >= 'a' && ch <= 'z') || ch == '|')
		return (true);
	return (false);
}

void Request::extractRequestHeaders()
{
	std::stringstream ss;
	if (_headerList.count("content-length"))
	{
		_bodyFlag = true;
		ss << _headerList["content-length"];
		ss >> _bodyLength;
	}
	if (_headerList.count("transfer-encoding"))
	{
		if (_headerList["transfer-encoding"].find_first_of("chunked") != std::string::npos)
			_chunkedFlag = true;
		_bodyFlag = true;
	}
	if (_headerList.count("host"))
	{
		size_t pos = _headerList["host"].find_first_of(':');
		_servName = _headerList["host"].substr(0, pos);
	}
	if (_headerList.count("content-type") && _headerList["content-type"].find("multipart/form-data") != std::string::npos)
	{
		size_t pos = _headerList["content-type"].find("boundary=", 0);
		if (pos != std::string::npos)
			this->_boundary = _headerList["content-type"].substr(pos + 9, _headerList["content-type"].size());
		this->_multiformFlag = true;
	}
}

void Request::setHeader(std::string &name, std::string &value)
{
	removeLeadingTrailingWhitespace(value);
	convertToLowerCase(name);
	_headerList[name] = value;
}

void Request::setMaxBodySize(size_t size)
{
	_maxBodySize = size;
}

void Request::setBody(std::string body)
{
	body.clear();
	body.insert(body.begin(), body.begin(), body.end());
	_bodyStr = body;
}

void Request::setErrorCode(short status)
{
	this->_errorCode = status;
}

short Request::errorCodes()
{
	return (this->_errorCode);
}

HttpMethod &Request::getHttpMethod()
{
	return (_httpMethod);
}

const std::map<std::string, std::string> &Request::getHeaders() const
{
	return (this->_headerList);
}

std::string &Request::getPath()
{
	return (_reqPath);
}

std::string &Request::getQuery()
{
	return (_query);
}

std::string Request::getHeader(std::string const &name)
{
	return (_headerList[name]);
}

std::string Request::getMethodStr()
{
	return (_httpMethodStr[_httpMethod]);
}

std::string &Request::getBody() {
	return (_bodyStr);
}

std::string Request::getServerName()
{
	return (this->_servName);
}

std::string &Request::getBoundary()
{
	return (this->_boundary);
}

bool Request::getMultiformFlag()
{
	return (this->_multiformFlag);
}

bool Request::isParsingDone()
{
	return (_parsStatus == STATE_PARSING_DONE);
}

bool Request::isConnectionKeepAlive()
{
	if (_headerList.count("connection"))
	{
		if (_headerList["connection"].find("close", 0) != std::string::npos)
			return (false);
	}
	return (true);
}

void Request::removeLeadingTrailingWhitespace(std::string &str)
{
	static const char *spaces = " \t";
	str.erase(0, str.find_first_not_of(spaces));
	str.erase(str.find_last_not_of(spaces) + 1);
}

void Request::convertToLowerCase(std::string &str)
{
	for (size_t i = 0; i < str.length(); ++i)
		str[i] = std::tolower(str[i]);
}

void Request::clear()
{
	_httpMethod = NONE;
	_parsStatus = STATE_REQUEST_LINE;
	_reqPath.clear();
	_query.clear();
	_bodyStr = "";
	_boundary.clear();
	_storage.clear();
	_heyStorage.clear();
	_servName.clear();
	_headerList.clear();
	_reqBody.clear();
	_bodyLength = 0;
	_chunkLen = 0x0;
	_httpMethodIndex = 1;
	_errorCode = 0;
	_bodyFlag = false;
	_bodyDoneFlag = false;
	_fieldsDoneFlag = false;
	_completeFlag = false;
	_chunkedFlag = false;
	_multiformFlag = false;
}
