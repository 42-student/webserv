#ifndef CGI_HPP
#define CGI_HPP

#include "utils.hpp"

class Location;

class Request;

class Cgi
{
	private:
			std::map<std::string, std::string>	_env;
			char**								_chEnv;
			char**								_av;
			int									_exitStatus;
			std::string							_cgiPath;
			pid_t								_cgiPid;

	public:
			int	pipeIn[2];
			int	pipeOut[2];		
	
			Cgi();
			Cgi(std::string path);
			Cgi(Cgi const &other);
			Cgi &operator=(Cgi const &rhs);
			~Cgi();

			void				setCgiPath(const std::string &_cgiPath);
			const pid_t			&getCgiPid() const;
			const std::string	&getCgiPath() const;
			std::string			getPathInfo(std::string &path, std::vector<std::string> extensions);
			void				setContentLength(int length);
			void				setContentType(const std::string &contentType);
			void				setDefaultEnvValues(Request &req, std::string cgiExec);
			void				setRequestHeaders(Request &req);
			void				createAv(const std::string &cgiExec);
			void				createChEnv();
			void				initEnv(Request &req, const std::vector<Location>::iterator it_loc);
			void				initEnvCgi(Request &req, const std::vector<Location>::iterator it_loc);
			void				execute(short &error_code);
			bool				isFileUpload();
			int					findStart(const std::string path, const std::string delim);
			std::string			decode(std::string &path);
			unsigned int		fromHexToDec(const std::string &nb);
			void				clear();
};

#endif
