#ifndef CGI_HPP
#define CGI_HPP

#include "utils.hpp"

class Location;

class Request;

class Cgi
{
	public:
			int		pipe_in[2];
			int		pipe_out[2];

			Cgi();
			Cgi(const std::string& script_path);
			Cgi(const Cgi& other);
			Cgi& operator=(const Cgi& rhs);
			~Cgi();

			void				set_script_path(const std::string& path);
			const pid_t&		get_process_id() const;
			const std::string&	get_script_path() const;
			void				initialize_environment(Request& request, const std::vector<Location>::iterator location);
			void				initialize_interpreter_environment(Request& request, const std::vector<Location>::iterator location);
			void				execute(short& error_code);
			bool				is_file_upload() const;
			std::string			extract_path_info(const std::string& path, const std::vector<std::string>& extensions) const;
			void				clear_resources();

	private:
			std::map<std::string, std::string>	environment_;
			char**								env_array_;
			char**								arg_array_;
			std::string							script_path_;
			pid_t								process_id_;
			int									exit_status_;

			void				set_common_environment_vars(const std::string& interpreter_path);
			void				set_content_variables(Request& request);
			void				set_server_variables(Request& request);
			void				set_request_headers(Request& request);
			void				build_argument_array(const std::string& interpreter_path);
			void				build_environment_array();
			std::string			decode_url(const std::string& encoded) const;
			unsigned int		hex_to_decimal(const std::string& hex) const;
			int					find_delimiter(const std::string& str, const std::string& delim) const;
			std::string			make_absolute_path(const std::string& relative_path) const;
};

#endif
