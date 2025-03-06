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

class Request
{
	private:
			HttpMethod							_httpMethod;
			int									_parsStatus;
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
            void										processChar(u_int8_t c);
            void										processRequestLine(u_int8_t c);
            void										processHeaders(u_int8_t c);
            void										processChunked(u_int8_t c);
            void										processMessageBody(u_int8_t c);
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
