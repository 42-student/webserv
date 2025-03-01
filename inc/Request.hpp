#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "utils.hpp"

enum HttpMethod
{
	GET,
	POST,
	DELETE,
	NONE
};

// enum ParsingState
// {
// 	REQUEST_LINE,
// 	REQUEST_LINE_POST_PUT,
// 	REQUEST_LINE_METHOD,
// 	REQUEST_LINE_FIRST_SPACE,
// 	REQUEST_LINE_URI_PATH_SLASH,
// 	REQUEST_LINE_URI_PATH,
// 	REQUEST_LINE_URI_QUERY,
// 	REQUEST_LINE_URI_FRAGMENT,
// 	REQUEST_LINE_VER,
// 	REQUEST_LINE_HT,
// 	REQUEST_LINE_HTT,
// 	REQUEST_LINE_HTTP,
// 	REQUEST_LINE_HTTP_SLASH,
// 	REQUEST_LINE_MAJOR,
// 	REQUEST_LINE_DOT,
// 	REQUEST_LINE_MINOR,
// 	REQUEST_LINE_CR,
// 	REQUEST_LINE_LF,
// 	FIELD_NAME_START,
// 	FIELDS_END,
// 	FIELD_NAME,
// 	FIELD_VALUE,
// 	FIELD_VALUE_END,
// 	CHUNKED_LENGTH_BEGIN,
// 	CHUNKED_LENGTH,
// 	CHUNKED_IGNORE,
// 	CHUNKED_LENGTH_CR,
// 	CHUNKED_LENGTH_LF,
// 	CHUNKED_DATA,
// 	CHUNKED_DATA_CR,
// 	CHUNKED_DATA_LF,
// 	CHUNKED_END_CR,
// 	CHUNKED_END_LF,
// 	MESSAGE_BODY,
// 	PARSING_DONE
// };

#define STATE_REQUEST_LINE 0
#define STATE_REQUEST_LINE_POST_PUT 1
#define STATE_REQUEST_LINE_METHOD 2
#define STATE_REQUEST_LINE_FIRST_SPACE 3
#define STATE_REQUEST_LINE_URI_PATH_SLASH 4
#define STATE_REQUEST_LINE_URI_PATH 5
#define STATE_REQUEST_LINE_URI_QUERY 6
#define STATE_REQUEST_LINE_URI_FRAGMENT 7
#define STATE_REQUEST_LINE_VER 8
#define STATE_REQUEST_LINE_HT 9
#define STATE_REQUEST_LINE_HTT 10
#define STATE_REQUEST_LINE_HTTP 11
#define STATE_REQUEST_LINE_HTTP_SLASH 12
#define STATE_REQUEST_LINE_MAJOR 13
#define STATE_REQUEST_LINE_DOT 14
#define STATE_REQUEST_LINE_MINOR 15
#define STATE_REQUEST_LINE_CR 16
#define STATE_REQUEST_LINE_LF 17
#define STATE_FIELD_NAME_START 18
#define STATE_FIELDS_END 19
#define STATE_FIELD_NAME 20
#define STATE_FIELD_VALUE 21
#define STATE_FIELD_VALUE_END 22
#define STATE_CHUNKED_LENGTH_BEGIN 23
#define STATE_CHUNKED_LENGTH 24
#define STATE_CHUNKED_IGNORE 25
#define STATE_CHUNKED_LENGTH_CR 26
#define STATE_CHUNKED_LENGTH_LF 27
#define STATE_CHUNKED_DATA 28
#define STATE_CHUNKED_DATA_CR 29
#define STATE_CHUNKED_DATA_LF 30
#define STATE_CHUNKED_END_CR 31
#define STATE_CHUNKED_END_LF 32
#define STATE_MESSAGE_BODY 33
#define STATE_PARSING_DONE 34

class Request
{
	private:
			HttpMethod							_httpMethod;
			//ParsingState						_parsStatus;
			int			_parsStatus;
			std::map<u_int8_t, std::string>		_httpMethodStr;
			std::string							_reqPath;
			std::string							_query;
			std::string							_bodyStr;
			std::string							_boundary;
			std::string							_storage;
			std::string							_heyStorage;
			std::string							_servName;
			std::map<std::string, std::string>	_headerList;
			std::vector<u_int8_t>				_reqBody;
			size_t								_maxBodySize;
			size_t								_bodyLength;
			size_t								_chunkLen;
			short								_httpMethodIndex;
			short								_errorCode;
			bool								_bodyFlag;
			bool								_bodyDoneFlag;
			bool								_fieldsDoneFlag;
			bool								_completeFlag;
			bool								_chunkedFlag;
			bool								_multiformFlag;
			u_int8_t							_verMaj;
			u_int8_t							_verMin;

	public:
			Request();
			~Request();
			
			void										parseHTTPRequestData(char *data, size_t size);
			bool										isValidUriPosition(std::string path);
			bool										isValidURIChar(uint8_t ch);
			bool										isValidTokenChar(uint8_t ch);
			void										extractRequestHeaders();
			void										setHeader(std::string &, std::string &);
			void										setMaxBodySize(size_t);
			void										setBody(std::string name);	
			void										setErrorCode(short status);
			short										errorCodes();
			HttpMethod									&getHttpMethod();
			const std::map<std::string, std::string>	&getHeaders() const;
			std::string									&getPath();
			std::string									&getQuery();
			std::string									getHeader(std::string const &);
			std::string									getMethodStr();
			std::string									&getBody();
			std::string									getServerName();
			std::string									&getBoundary();
			bool										getMultiformFlag();
			bool										isParsingDone();
			bool										isConnectionKeepAlive();
			void										removeLeadingTrailingWhitespace(std::string &str);
			void										convertToLowerCase(std::string &str);
			void										clear();
};

#endif
