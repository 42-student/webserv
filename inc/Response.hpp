#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "Location.hpp"
#include "Request.hpp"
#include "utils.hpp"

class Location;

class Response
{
	private:
			Server								_serv;
			std::string							_targetFile;
			std::string							_respBody;
			std::string							_loc;
			std::vector<uint8_t>				_body;
			size_t								_bodyLen;
			int									_cgi;
			int									_cgiFd[2];
			size_t								_cgiRespLen;
			bool								_autoIndex;
			short								_code;
			std::map<std::string, std::string>	_mimeType;

	public:
			Request 		request;
			Cgi 			cgiObj;
			std::string 	responseContent;
			
			Response();
			Response(Request &);
			~Response();
			
			void			buildResponse();
			bool			reqError();
			int				buildBody();
			void			buildErrorBody();
			int				buildHtmlIndex(std::string &dirName, std::vector<uint8_t> &body, size_t &bodyLen);
			bool			isBodySizeExceeded();
			int				handleTarget();
			bool			isMethodNotAllowed(Location &location);
			bool			isAllowedMethod(int method, Location &location, short &code);
			bool			isRequestBodySizeExceeded(const std::string &body, const Location &location);
			bool			checkLocationReturn(Location &location);
			bool 			checkReturn(Location &loc, short &code, std::string &location);
			bool			checkLocationRedirect(Location &location);
			bool 			checkRedirect(Location &loc, short &code, std::string &location);
			bool			isCgiPath(const std::string &path);
			int				handleCgi(std::string &);
			bool			isValidPath(std::string &path, std::string &locationKey, size_t &pos);
			bool			isValidExtension(std::string &path);
			bool			isValidFileType(std::string &path);
			bool			isFileAllowed(Request &request, std::string &locationKey);
			bool			initializeCgi(std::string &path, std::string &locationKey);
			void			replaceAlias(Location &location, Request &request, std::string &_targetFile);
			void			appendRoot(Location &location, Request &request, std::string &_targetFile);
			std::string		combinePaths(std::string p1, std::string p2, std::string p3);
			int				controllerCgiTemp(std::string &);
			bool			isDirectory(std::string path);
			bool			handleIndexLocation(const std::string &indexLocation, bool autoindex);
			bool 			fileExists(const std::string &f);
			bool			handleNonLocation(const std::string &root, Request &request);
			void			setRequest(Request &);
			void			setServer(Server &);
			void			setStatusLine();
			void			setHeaders();
			void			setServerDefaultErrorPages();
			void			setErrorResponse(short code);
			void			setCgiState(int);
			void			getLocationMatch(std::string &path, std::vector<Location> locations, std::string &locationKey);
			std::string		getRes();
			size_t			getLen() const;
			int				getCode() const;
			int				getCgiState();
			std::string		getErrorPage(short);
			std::string		getMimeType(std::string extension) const;	
			bool			hasMimeType(std::string &extension) const;
			bool			isBoundaryLine(const std::string &line, const std::string &boundary);
			bool			isEndBoundaryLine(const std::string &line, const std::string &boundary);
			bool			isCgiExtension(const std::string &_targetFile, const Location &location);
			bool			handleTgt();
			bool			handleGetMethod();
			bool			handlePostMethod();
			bool			handleDeleteMethod();
			int				readFile();
			void			contentType();
			void			contentLength();
			void			connection();
			void			server();
			void			locations();
			void			date();
			std::string		removeBoundary(std::string &body, const std::string &boundary);
			std::string		extractFilename(const std::string &line);
			void			cutRes(size_t);
			void			clear();
};

#endif
