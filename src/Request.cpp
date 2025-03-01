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

// void Request::parseHTTPRequestData(char *data, size_t size)
// {
// 	u_int8_t character;
// 	static std::stringstream s;
// 	for (size_t i = 0; i < size; ++i)
// 	{
// 		character = data[i];
// 		switch (_parsStatus)
// 		{
// 			case REQUEST_LINE:
// 			{
// 				if (character == 'G')
// 					_httpMethod = GET;
// 				else if (character == 'P')
// 				{
// 					_parsStatus = REQUEST_LINE_POST_PUT;
// 					break;
// 				}
// 				else if (character == 'D')
// 					_httpMethod = DELETE;
// 				else if (character == 'H')
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 501, "Unsupported method <%s>.", "HEAD");
// 					return;
// 				}
// 				else if (character == 'O')
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 501, "Unsupported method <%s>.", "OPTIONS");
// 					return;
// 				}
// 				else if (character == 'U')
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 501, "Unsupported method <%s>.", "UNKNOWN");
// 					return;
// 				}
// 				else
// 				{
// 					PrintApp::printEvent(ORANGE, SUCCESS, "Invalid character \"%c\".", character);
// 					return;
// 				}
// 				_parsStatus = REQUEST_LINE_METHOD;
// 				break;
// 			}
// 			case REQUEST_LINE_POST_PUT:
// 			{
// 				if (character == 'O')
// 					_httpMethod = POST;
// 				else if (character == 'U')
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 501, "Unsupported method <%s>.", "PUT");
// 					return;
// 				}
// 				else if (character == 'A')
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 501, "Unsupported method <%s>.", "PATCH");
// 					return;
// 				}
// 				else
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 501, "Invalid character \"%c\".", character);
// 					return;
// 				}
// 				_httpMethodIndex++;
// 				_parsStatus = REQUEST_LINE_METHOD;
// 				break;
// 			}
// 			case REQUEST_LINE_METHOD:
// 			{
// 				if (character == _httpMethodStr[_httpMethod][_httpMethodIndex])
// 					_httpMethodIndex++;
// 				else
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 501, "Invalid character \"%c\".", character);
// 					return;
// 				}	
// 				if ((size_t)_httpMethodIndex == _httpMethodStr[_httpMethod].length())
// 					_parsStatus = REQUEST_LINE_FIRST_SPACE;
// 				break;
// 			}
// 			case REQUEST_LINE_FIRST_SPACE:
// 			{
// 				if (character != ' ')
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
// 					return;
// 				}
// 				_parsStatus = REQUEST_LINE_URI_PATH_SLASH;
// 				continue;
// 			}
// 			case REQUEST_LINE_URI_PATH_SLASH:
// 			{
// 				if (character == '/')
// 				{
// 					_parsStatus = REQUEST_LINE_URI_PATH;
// 					_storage.clear();
// 				}
// 				else
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
// 					return;
// 				}
// 				break;
// 			}
// 			case REQUEST_LINE_URI_PATH:
// 			{
// 				if (character == ' ')
// 				{
// 					_parsStatus = REQUEST_LINE_VER;
// 					_reqPath.append(_storage);
// 					_storage.clear();
// 					continue;
// 				}
// 				else if (character == '?')
// 				{
// 					_parsStatus = REQUEST_LINE_URI_QUERY;
// 					_reqPath.append(_storage);
// 					_storage.clear();
// 					continue;
// 				}
// 				else if (character == '#')
// 				{
// 					_parsStatus = REQUEST_LINE_URI_FRAGMENT;
// 					_reqPath.append(_storage);
// 					_storage.clear();
// 					continue;
// 				}
// 				else if (!isValidURIChar(character))
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
// 					return;
// 				}
// 				else if (i > MAX_URI_LENGTH)
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 414, "URI exceeds maximum length: \"%c\".", character);
// 					return;
// 				}
// 				break;
// 			}
// 			case REQUEST_LINE_URI_QUERY:
// 			{
// 				if (character == ' ')
// 				{
// 					_parsStatus = REQUEST_LINE_VER;
// 					_query.append(_storage);
// 					_storage.clear();
// 					continue;
// 				}
// 				else if (character == '#')
// 				{
// 					_parsStatus = REQUEST_LINE_URI_FRAGMENT;
// 					_query.append(_storage);
// 					_storage.clear();
// 					continue;
// 				}
// 				else if (!isValidURIChar(character))
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
// 					return;
// 				}
// 				else if (i > MAX_URI_LENGTH)
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 414, "URI exceeds maximum length: \"%c\".", character);
// 					return;
// 				}
// 				break;
// 			}
// 			case REQUEST_LINE_URI_FRAGMENT:
// 			{
// 				if (character == ' ')
// 				{
// 					_parsStatus = REQUEST_LINE_VER;
// 					_storage.clear();
// 					continue;
// 				}
// 				else if (!isValidURIChar(character))
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
// 					return;
// 				}
// 				else if (i > MAX_URI_LENGTH)
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 414, "URI exceeds maximum length: \"%c\".", character);
// 					return;
// 				}
// 				break;
// 			}
// 			case REQUEST_LINE_VER:
// 			{
// 				if (isValidUriPosition(_reqPath))
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
// 					return;
// 				}
// 				if (character != 'H')
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
// 					return;
// 				}
// 				_parsStatus = REQUEST_LINE_HT;
// 				break;
// 			}
// 			case REQUEST_LINE_HT:
// 			{
// 				if (character != 'T')
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
// 					return;
// 				}
// 				_parsStatus = REQUEST_LINE_HTT;
// 				break;
// 			}
// 			case REQUEST_LINE_HTT:
// 			{
// 				if (character != 'T')
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
// 					return;
// 				}
// 				_parsStatus = REQUEST_LINE_HTTP;
// 				break;
// 			}
// 			case REQUEST_LINE_HTTP:
// 			{
// 				if (character != 'P')
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
// 					return;
// 				}
// 				_parsStatus = REQUEST_LINE_HTTP_SLASH;
// 				break;
// 			}
// 			case REQUEST_LINE_HTTP_SLASH:
// 			{
// 				if (character != '/')
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
// 					return;
// 				}
// 				_parsStatus = REQUEST_LINE_MAJOR;
// 				break;
// 			}
// 			case REQUEST_LINE_MAJOR:
// 			{
// 				if (!isdigit(character))
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
// 					return;
// 				}
// 				_verMaj = character;	
// 				_parsStatus = REQUEST_LINE_DOT;
// 				break;
// 			}
// 			case REQUEST_LINE_DOT:
// 			{
// 				if (character != '.')
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
// 					return;
// 				}
// 				_parsStatus = REQUEST_LINE_MINOR;
// 				break;
// 			}
// 			case REQUEST_LINE_MINOR:
// 			{
// 				if (!isdigit(character))
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
// 					return;
// 				}
// 				_verMin = character;
// 				_parsStatus = REQUEST_LINE_CR;
// 				break;
// 			}
// 			case REQUEST_LINE_CR:
// 			{
// 				if (character != '\r')
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
// 					return;
// 				}
// 				_parsStatus = REQUEST_LINE_LF;
// 				break;
// 			}
// 			case REQUEST_LINE_LF:
// 			{
// 				if (character != '\n')
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
// 					return;
// 				}
// 				_parsStatus = FIELD_NAME_START;
// 				_storage.clear();
// 				continue;
// 			}
// 			case FIELD_NAME_START:
// 			{
// 				if (character == '\r')
// 					_parsStatus = FIELDS_END;
// 				else if (isValidTokenChar(character))
// 					_parsStatus = FIELD_NAME;
// 				else
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
// 					return;
// 				}
// 				break;
// 			}
// 			case FIELDS_END:
// 			{
// 				if (character == '\n')
// 				{
// 					_storage.clear();
// 					_fieldsDoneFlag = true;
// 					extractRequestHeaders();
// 					if (_bodyFlag == 1)
// 					{
// 						if (_chunkedFlag == true)
// 							_parsStatus = CHUNKED_LENGTH_BEGIN;
// 						else
// 							_parsStatus = MESSAGE_BODY;
// 					}
// 					else
// 						_parsStatus = PARSING_DONE;
// 					continue;
// 				}
// 				else
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
// 					return;
// 				}
// 				break;
// 			}
// 			case FIELD_NAME:
// 			{
// 				if (character == ':')
// 				{
// 					_heyStorage = _storage;
// 					_storage.clear();
// 					_parsStatus = FIELD_VALUE;
// 					continue;
// 				}
// 				else if (!isValidTokenChar(character))
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
// 					return;
// 				}
// 				break;
// 			}
// 			case FIELD_VALUE:
// 			{
// 				if (character == '\r')
// 				{
// 					setHeader(_heyStorage, _storage);
// 					_heyStorage.clear();
// 					_storage.clear();
// 					_parsStatus = FIELD_VALUE_END;
// 					continue;
// 				}
// 				break;
// 			}
// 			case FIELD_VALUE_END:
// 			{
// 				if (character == '\n')
// 				{
// 					_parsStatus = FIELD_NAME_START;
// 					continue;
// 				}
// 				else
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
// 					return;
// 				}
// 				break;
// 			}
// 			case CHUNKED_LENGTH_BEGIN:
// 			{
// 				if (isxdigit(character) == 0)
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
// 					return;
// 				}
// 				s.str("");
// 				s.clear();
// 				s << character;
// 				s >> std::hex >> _chunkLen;
// 				if (_chunkLen == 0)
// 					_parsStatus = CHUNKED_LENGTH_CR;
// 				else
// 					_parsStatus = CHUNKED_LENGTH;
// 				continue;
// 			}
// 			case CHUNKED_LENGTH:
// 			{
// 				if (isxdigit(character) != 0)
// 				{
// 					int temp_len = 0;
// 					s.str("");
// 					s.clear();
// 					s << character;
// 					s >> std::hex >> temp_len;
// 					_chunkLen *= 16;
// 					_chunkLen += temp_len;
// 				}
// 				else if (character == '\r')
// 					_parsStatus = CHUNKED_END_LF;
// 				else
// 					_parsStatus = CHUNKED_IGNORE;
// 				continue;
// 			}
// 			case CHUNKED_LENGTH_CR:
// 			{
// 				if (character == '\r')
// 					_parsStatus = CHUNKED_END_LF;
// 				else
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
// 					return;
// 				}
// 				continue;
// 			}
// 			case CHUNKED_LENGTH_LF:
// 			{
// 				if (character == '\n')
// 				{
// 					if (_chunkLen == 0)
// 						_parsStatus = CHUNKED_END_CR;
// 					else
// 						_parsStatus = CHUNKED_DATA;
// 				} else
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
// 					return;
// 				}
// 				continue;
// 			}
// 			case CHUNKED_IGNORE:
// 			{
// 				if (character == '\r')
// 					_parsStatus = CHUNKED_END_LF;
// 				continue;
// 			}
// 			case CHUNKED_DATA:
// 			{
// 				_reqBody.push_back(character);
// 				--_chunkLen;
// 				if (_chunkLen == 0)
// 					_parsStatus = CHUNKED_DATA_CR;
// 				continue;
// 			}
// 			case CHUNKED_DATA_CR:
// 			{
// 				if (character == '\r')
// 					_parsStatus = CHUNKED_DATA_LF;
// 				else
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
// 					return;
// 				}
// 				continue;
// 			}
// 			case CHUNKED_DATA_LF:
// 			{
// 				if (character == '\n')
// 					_parsStatus = CHUNKED_LENGTH_BEGIN;
// 				else
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
// 					return;
// 				}
// 				continue;
// 			}
// 			case CHUNKED_END_CR:
// 			{
// 				if (character != '\r')
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
// 					return;
// 				}
// 				_parsStatus = CHUNKED_END_LF;
// 				continue;
// 			}
// 			case CHUNKED_END_LF:
// 			{
// 				if (character != '\n')
// 				{
// 					PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
// 					return;
// 				}
// 				_bodyDoneFlag = true;
// 				_parsStatus = PARSING_DONE;
// 				continue;
// 			}
// 			case MESSAGE_BODY:
// 			{
// 				if (_reqBody.size() < _bodyLength)
// 				_reqBody.push_back(character);
// 				if (_reqBody.size() == _bodyLength)
// 				{
// 					_bodyDoneFlag = true;
// 					_parsStatus = PARSING_DONE;
// 				}
// 				break;
// 			}
// 			case PARSING_DONE:
// 			{
// 				return;
// 			}
// 		}
// 		_storage += character;
// 	}
// 	if (_parsStatus == PARSING_DONE)
// 	{
// 		_bodyStr.append((char *)_reqBody.data(), _reqBody.size());
// 	}
// }

// Implementation
void Request::parseHTTPRequestData(char *data, size_t size)
{
    u_int8_t character;
    static std::stringstream s;
    
    for (size_t i = 0; i < size; ++i)
    {
        character = data[i];
        
        if (_parsStatus == STATE_REQUEST_LINE)
        {
            if (character == 'G')
                _httpMethod = GET;
            else if (character == 'P')
            {
                _parsStatus = STATE_REQUEST_LINE_POST_PUT;
                continue;
            }
            else if (character == 'D')
                _httpMethod = DELETE;
            else if (character == 'H')
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 501, "Unsupported method <%s>.", "HEAD");
                return;
            }
            else if (character == 'O')
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 501, "Unsupported method <%s>.", "OPTIONS");
                return;
            }
            else if (character == 'U')
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 501, "Unsupported method <%s>.", "UNKNOWN");
                return;
            }
            else
            {
                PrintApp::printEvent(ORANGE, SUCCESS, "Invalid character \"%c\".", character);
                return;
            }
            _parsStatus = STATE_REQUEST_LINE_METHOD;
        }
        else if (_parsStatus == STATE_REQUEST_LINE_POST_PUT)
        {
            if (character == 'O')
                _httpMethod = POST;
            else if (character == 'U')
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 501, "Unsupported method <%s>.", "PUT");
                return;
            }
            else if (character == 'A')
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 501, "Unsupported method <%s>.", "PATCH");
                return;
            }
            else
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 501, "Invalid character \"%c\".", character);
                return;
            }
            _httpMethodIndex++;
            _parsStatus = STATE_REQUEST_LINE_METHOD;
        }
        else if (_parsStatus == STATE_REQUEST_LINE_METHOD)
        {
            if (character == _httpMethodStr[_httpMethod][_httpMethodIndex])
                _httpMethodIndex++;
            else
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 501, "Invalid character \"%c\".", character);
                return;
            }   
            if ((size_t)_httpMethodIndex == _httpMethodStr[_httpMethod].length())
                _parsStatus = STATE_REQUEST_LINE_FIRST_SPACE;
        }
        else if (_parsStatus == STATE_REQUEST_LINE_FIRST_SPACE)
        {
            if (character != ' ')
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
                return;
            }
            _parsStatus = STATE_REQUEST_LINE_URI_PATH_SLASH;
            continue;
        }
        else if (_parsStatus == STATE_REQUEST_LINE_URI_PATH_SLASH)
        {
            if (character == '/')
            {
                _parsStatus = STATE_REQUEST_LINE_URI_PATH;
                _storage.clear();
            }
            else
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
                return;
            }
        }
        else if (_parsStatus == STATE_REQUEST_LINE_URI_PATH)
        {
            if (character == ' ')
            {
                _parsStatus = STATE_REQUEST_LINE_VER;
                _reqPath.append(_storage);
                _storage.clear();
                continue;
            }
            else if (character == '?')
            {
                _parsStatus = STATE_REQUEST_LINE_URI_QUERY;
                _reqPath.append(_storage);
                _storage.clear();
                continue;
            }
            else if (character == '#')
            {
                _parsStatus = STATE_REQUEST_LINE_URI_FRAGMENT;
                _reqPath.append(_storage);
                _storage.clear();
                continue;
            }
            else if (!isValidURIChar(character))
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
                return;
            }
            else if (i > MAX_URI_LENGTH)
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 414, "URI exceeds maximum length: \"%c\".", character);
                return;
            }
        }
        else if (_parsStatus == STATE_REQUEST_LINE_URI_QUERY)
        {
            if (character == ' ')
            {
                _parsStatus = STATE_REQUEST_LINE_VER;
                _query.append(_storage);
                _storage.clear();
                continue;
            }
            else if (character == '#')
            {
                _parsStatus = STATE_REQUEST_LINE_URI_FRAGMENT;
                _query.append(_storage);
                _storage.clear();
                continue;
            }
            else if (!isValidURIChar(character))
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
                return;
            }
            else if (i > MAX_URI_LENGTH)
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 414, "URI exceeds maximum length: \"%c\".", character);
                return;
            }
        }
        else if (_parsStatus == STATE_REQUEST_LINE_URI_FRAGMENT)
        {
            if (character == ' ')
            {
                _parsStatus = STATE_REQUEST_LINE_VER;
                _storage.clear();
                continue;
            }
            else if (!isValidURIChar(character))
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
                return;
            }
            else if (i > MAX_URI_LENGTH)
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 414, "URI exceeds maximum length: \"%c\".", character);
                return;
            }
        }
        else if (_parsStatus == STATE_REQUEST_LINE_VER)
        {
            if (isValidUriPosition(_reqPath))
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
                return;
            }
            if (character != 'H')
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
                return;
            }
            _parsStatus = STATE_REQUEST_LINE_HT;
        }
        else if (_parsStatus == STATE_REQUEST_LINE_HT)
        {
            if (character != 'T')
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
                return;
            }
            _parsStatus = STATE_REQUEST_LINE_HTT;
        }
        else if (_parsStatus == STATE_REQUEST_LINE_HTT)
        {
            if (character != 'T')
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
                return;
            }
            _parsStatus = STATE_REQUEST_LINE_HTTP;
        }
        else if (_parsStatus == STATE_REQUEST_LINE_HTTP)
        {
            if (character != 'P')
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
                return;
            }
            _parsStatus = STATE_REQUEST_LINE_HTTP_SLASH;
        }
        else if (_parsStatus == STATE_REQUEST_LINE_HTTP_SLASH)
        {
            if (character != '/')
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
                return;
            }
            _parsStatus = STATE_REQUEST_LINE_MAJOR;
        }
        else if (_parsStatus == STATE_REQUEST_LINE_MAJOR)
        {
            if (!isdigit(character))
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
                return;
            }
            _verMaj = character;    
            _parsStatus = STATE_REQUEST_LINE_DOT;
        }
        else if (_parsStatus == STATE_REQUEST_LINE_DOT)
        {
            if (character != '.')
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
                return;
            }
            _parsStatus = STATE_REQUEST_LINE_MINOR;
        }
        else if (_parsStatus == STATE_REQUEST_LINE_MINOR)
        {
            if (!isdigit(character))
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
                return;
            }
            _verMin = character;
            _parsStatus = STATE_REQUEST_LINE_CR;
        }
        else if (_parsStatus == STATE_REQUEST_LINE_CR)
        {
            if (character != '\r')
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
                return;
            }
            _parsStatus = STATE_REQUEST_LINE_LF;
        }
        else if (_parsStatus == STATE_REQUEST_LINE_LF)
        {
            if (character != '\n')
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
                return;
            }
            _parsStatus = STATE_FIELD_NAME_START;
            _storage.clear();
            continue;
        }
        else if (_parsStatus == STATE_FIELD_NAME_START)
        {
            if (character == '\r')
                _parsStatus = STATE_FIELDS_END;
            else if (isValidTokenChar(character))
                _parsStatus = STATE_FIELD_NAME;
            else
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
                return;
            }
        }
        else if (_parsStatus == STATE_FIELDS_END)
        {
            if (character == '\n')
            {
                _storage.clear();
                _fieldsDoneFlag = true;
                extractRequestHeaders();
                if (_bodyFlag == 1)
                {
                    if (_chunkedFlag == true)
                        _parsStatus = STATE_CHUNKED_LENGTH_BEGIN;
                    else
                        _parsStatus = STATE_MESSAGE_BODY;
                }
                else
                    _parsStatus = STATE_PARSING_DONE;
                continue;
            }
            else
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
                return;
            }
        }
        else if (_parsStatus == STATE_FIELD_NAME)
        {
            if (character == ':')
            {
                _heyStorage = _storage;
                _storage.clear();
                _parsStatus = STATE_FIELD_VALUE;
                continue;
            }
            else if (!isValidTokenChar(character))
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
                return;
            }
        }
        else if (_parsStatus == STATE_FIELD_VALUE)
        {
            if (character == '\r')
            {
                setHeader(_heyStorage, _storage);
                _heyStorage.clear();
                _storage.clear();
                _parsStatus = STATE_FIELD_VALUE_END;
                continue;
            }
        }
        else if (_parsStatus == STATE_FIELD_VALUE_END)
        {
            if (character == '\n')
            {
                _parsStatus = STATE_FIELD_NAME_START;
                continue;
            }
            else
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
                return;
            }
        }
        else if (_parsStatus == STATE_CHUNKED_LENGTH_BEGIN)
        {
            if (isxdigit(character) == 0)
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
                return;
            }
            s.str("");
            s.clear();
            s << character;
            s >> std::hex >> _chunkLen;
            if (_chunkLen == 0)
                _parsStatus = STATE_CHUNKED_LENGTH_CR;
            else
                _parsStatus = STATE_CHUNKED_LENGTH;
            continue;
        }
        else if (_parsStatus == STATE_CHUNKED_LENGTH)
        {
            if (isxdigit(character) != 0)
            {
                int temp_len = 0;
                s.str("");
                s.clear();
                s << character;
                s >> std::hex >> temp_len;
                _chunkLen *= 16;
                _chunkLen += temp_len;
            }
            else if (character == '\r')
                _parsStatus = STATE_CHUNKED_END_LF;
            else
                _parsStatus = STATE_CHUNKED_IGNORE;
            continue;
        }
        else if (_parsStatus == STATE_CHUNKED_LENGTH_CR)
        {
            if (character == '\r')
                _parsStatus = STATE_CHUNKED_END_LF;
            else
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
                return;
            }
            continue;
        }
        else if (_parsStatus == STATE_CHUNKED_LENGTH_LF)
        {
            if (character == '\n')
            {
                if (_chunkLen == 0)
                    _parsStatus = STATE_CHUNKED_END_CR;
                else
                    _parsStatus = STATE_CHUNKED_DATA;
            }
            else
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
                return;
            }
            continue;
        }
        else if (_parsStatus == STATE_CHUNKED_IGNORE)
        {
            if (character == '\r')
                _parsStatus = STATE_CHUNKED_END_LF;
            continue;
        }
        else if (_parsStatus == STATE_CHUNKED_DATA)
        {
            _reqBody.push_back(character);
            --_chunkLen;
            if (_chunkLen == 0)
                _parsStatus = STATE_CHUNKED_DATA_CR;
            continue;
        }
        else if (_parsStatus == STATE_CHUNKED_DATA_CR)
        {
            if (character == '\r')
                _parsStatus = STATE_CHUNKED_DATA_LF;
            else
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
                return;
            }
            continue;
        }
        else if (_parsStatus == STATE_CHUNKED_DATA_LF)
        {
            if (character == '\n')
                _parsStatus = STATE_CHUNKED_LENGTH_BEGIN;
            else
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
                return;
            }
            continue;
        }
        else if (_parsStatus == STATE_CHUNKED_END_CR)
        {
            if (character != '\r')
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
                return;
            }
            _parsStatus = STATE_CHUNKED_END_LF;
            continue;
        }
        else if (_parsStatus == STATE_CHUNKED_END_LF)
        {
            if (character != '\n')
            {
                PrintApp::printErrorCode(ORANGE, _errorCode, 400, "Unexpected character \"%c\" found.", character);
                return;
            }
            _bodyDoneFlag = true;
            _parsStatus = STATE_PARSING_DONE;
            continue;
        }
        else if (_parsStatus == STATE_MESSAGE_BODY)
        {
            if (_reqBody.size() < _bodyLength)
                _reqBody.push_back(character);
            if (_reqBody.size() == _bodyLength)
            {
                _bodyDoneFlag = true;
                _parsStatus = STATE_PARSING_DONE;
            }
        }
        else if (_parsStatus == STATE_PARSING_DONE)
        {
            return;
        }
        
        _storage += character;
    }
    
    if (_parsStatus == STATE_PARSING_DONE)
    {
        _bodyStr.append((char *)_reqBody.data(), _reqBody.size());
    }
}

////////////////////////////////////////////////////////////////

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
