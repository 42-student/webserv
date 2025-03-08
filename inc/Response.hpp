#ifndef RESPONSE_H
#define RESPONSE_H

#include "utils.hpp"

class Location;

class Response
{
	private:
		Server								server_;
		std::string							target_file_;
		std::string							body_content_;
		std::string							redirect_loc_;
		std::vector<unsigned char>			body_data_;
		size_t								body_size_;
		int									cgi_flag_;
		int									cgi_pipe_[2];
		size_t								cgi_response_size_;
		bool								auto_index_;
		short								status_code_;
		std::map<std::string, std::string>	mime_types_;

		void setupMimeTypes();

	public:
		Request			req_;
		Cgi				cgi_handler_;
		std::string		full_response_;

		Response();
		Response(Request& req);
		~Response();

		void			constructResponse();
		bool			hasRequestError();
		int				prepareBody();
		void			generateErrorPage();
		int				createDirListing(const std::string& dir, std::vector<unsigned char>& data, size_t& size);
		bool			exceedsBodyLimit();
		int				processTarget();
		bool			methodDisallowed(Location& loc);
		bool			checkMethod(int method, Location& loc, short& code);
		bool			bodyTooLarge(const std::string& body, const Location& loc);
		bool			hasReturn(Location& loc);
		bool			evalReturn(Location& loc, short& code, std::string& redirect);
		bool			hasRedirect(Location& loc);
		bool			evalRedirect(Location& loc, short& code, std::string& redirect);
		bool			isCgi(const std::string& path);
		int				manageCgi(std::string& loc_key);
		bool			validatePath(std::string& path, std::string& loc_key, size_t& dot_pos);
		bool			validExt(const std::string& path);
		bool			validFile(const std::string& path);
		bool			filePermitted(Request& req, std::string& loc_key);
		bool			initCgi(const std::string& path, std::string& loc_key);
		void			applyAlias(Location& loc, Request& req, std::string& target);
		void			addRoot(Location& loc, Request& req, std::string& target);
		std::string		joinPaths(const std::string& p1, const std::string& p2, const std::string& p3);
		int				handleTempCgi(std::string& loc_key);
		bool			isDir(const std::string& path);
		bool			processIndex(const std::string& index, bool autoindex);
		bool			exists(const std::string& file);
		bool			handleNoLocation(const std::string& root, Request& req);
		void			setRequest(Request& req);
		void			setServer(Server& srv);
		void			writeStatus();
		void			writeHeaders();
		void			setDefaultError();
		void			setStatus(short code);
		void			setError(short code);
		void			toggleCgi(int state);
		void			findLocation(const std::string& path, const std::vector<Location>& locs, std::string& key);
		std::string		getResponse();
		size_t			getLength() const;
		int				getStatus() const;
		int				getCgiFlag();
		std::string		makeErrorPage(short code);
		std::string		fetchMimeType(const std::string& ext) const;
		bool			knowsMimeType(const std::string& ext) const;
		bool			isBoundary(const std::string& line, const std::string& boundary);
		bool			isEndBoundary(const std::string& line, const std::string& boundary);
		bool			matchesCgiExt(const std::string& target, const Location& loc);
		bool			skipBody();
		bool			doGet();
		bool			doPost();
		bool			doDelete();
		int				loadFile();
		void			addContentType();
		void			addContentLength();
		void			addConnection();
		void			addServer();
		void			addLocation();
		void			addDate();
		std::string		stripBoundary(std::string& body, const std::string& boundary);
		std::string		getFilename(const std::string& line);
		void			trimResponse(size_t pos);
		void			reset();
};

#endif
