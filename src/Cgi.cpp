#include "../inc/Cgi.hpp"

Cgi::Cgi() : env_array_(NULL), arg_array_(NULL), script_path_(""), process_id_(-1), exit_status_(0)
{
	pipe_in[0] = pipe_in[1] = pipe_out[0] = pipe_out[1] = -1;
}

Cgi::Cgi(const std::string& script_path) : env_array_(NULL), arg_array_(NULL), script_path_(script_path), process_id_(-1), exit_status_(0)
{
	pipe_in[0] = pipe_in[1] = pipe_out[0] = pipe_out[1] = -1;
}

Cgi::Cgi(const Cgi& other) : environment_(other.environment_), env_array_(NULL), arg_array_(NULL),
	script_path_(other.script_path_), process_id_(other.process_id_), exit_status_(other.exit_status_)
{
	pipe_in[0] = pipe_in[1] = pipe_out[0] = pipe_out[1] = -1;
}

Cgi& Cgi::operator=(const Cgi& rhs)
{
	if (this != &rhs)
	{
		clear_resources();
		environment_ = rhs.environment_;
		env_array_ = NULL;
		arg_array_ = NULL;
		script_path_ = rhs.script_path_;
		process_id_ = rhs.process_id_;
		exit_status_ = rhs.exit_status_;
	}
	return *this;
}

Cgi::~Cgi()
{
	clear_resources();
}

void Cgi::set_script_path(const std::string& path)
{
	script_path_ = path;
}

const pid_t& Cgi::get_process_id() const
{
	return process_id_;
}

const std::string& Cgi::get_script_path() const
{
	return script_path_;
}

std::string Cgi::extract_path_info(const std::string& path, const std::vector<std::string>& extensions) const
{
	std::string result;
	size_t ext_pos = std::string::npos;
	std::vector<std::string>::const_iterator it;
	for (it = extensions.begin(); it != extensions.end(); ++it)
	{
		ext_pos = path.find(*it);
		if (ext_pos != std::string::npos)
			break;
	}
	if (ext_pos == std::string::npos || ext_pos + it->length() >= path.size())
		return "";
	result = path.substr(ext_pos + it->length());
	if (result.empty() || result[0] != '/')
		return "";
	size_t query_pos = result.find('?');
	return (query_pos == std::string::npos) ? result : result.substr(0, query_pos);
}

void Cgi::set_common_environment_vars(const std::string&)
{
	environment_["GATEWAY_INTERFACE"] = "CGI/1.1";
	environment_["SERVER_PROTOCOL"] = "HTTP/1.1";
	environment_["REDIRECT_STATUS"] = "200";
	environment_["SERVER_SOFTWARE"] = "miau";
	environment_["SCRIPT_FILENAME"] = script_path_;
	environment_["SCRIPT_NAME"] = script_path_;
}

void Cgi::set_content_variables(Request& request)
{
	if (!request.getBody().empty())
	{
		std::stringstream ss;
		ss << request.getBody().length();
		environment_["CONTENT_LENGTH"] = ss.str();
		std::string content_type = request.getHeader("content-type");
		if (!content_type.empty())
			environment_["CONTENT_TYPE"] = content_type;
	}
}

void Cgi::set_server_variables(Request& request)
{
	std::string host = request.getHeader("host");
	size_t colon_pos = host.find(':');
	if (colon_pos != std::string::npos)
	{
		environment_["SERVER_NAME"] = host.substr(0, colon_pos);
		environment_["SERVER_PORT"] = host.substr(colon_pos + 1);
	}
	else
	{
		environment_["SERVER_NAME"] = host;
		environment_["SERVER_PORT"] = "80";
	}
	environment_["REQUEST_METHOD"] = request.getMethodStr();
	environment_["REMOTE_ADDR"] = host;
}

void Cgi::set_request_headers(Request& request)
{
	std::map<std::string, std::string> headers = request.getHeaders();
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
	{
		std::string key = it->first;
		std::transform(key.begin(), key.end(), key.begin(), ::toupper);
		environment_["HTTP_" + key] = it->second;
	}
}

void Cgi::build_argument_array(const std::string& interpreter_path)
{
	arg_array_ = new char*[3];
	arg_array_[0] = new char[interpreter_path.length() + 1];
	std::strcpy(arg_array_[0], interpreter_path.c_str());
	arg_array_[1] = new char[script_path_.length() + 1];
	std::strcpy(arg_array_[1], script_path_.c_str());
	arg_array_[2] = NULL;
}

void Cgi::build_environment_array()
{
	env_array_ = new char*[environment_.size() + 1];
	int i = 0;
	for (std::map<std::string, std::string>::const_iterator it = environment_.begin(); it != environment_.end(); ++it)
	{
		std::string env_var = it->first + "=" + it->second;
		env_array_[i] = new char[env_var.length() + 1];
		std::strcpy(env_array_[i], env_var.c_str());
		++i;
	}
	env_array_[i] = NULL;
}

void Cgi::initialize_environment(Request& request, const std::vector<Location>::iterator location)
{
	std::string extension = script_path_.substr(script_path_.find('.'));
	std::map<std::string, std::string>::iterator it = location->extPath.find(extension);
	if (it == location->extPath.end())
		return;
	std::string interpreter_path = it->second;
	environment_["AUTH_TYPE"] = "Basic";
	set_content_variables(request);
	set_common_environment_vars(interpreter_path);
	set_server_variables(request);
	set_request_headers(request);
	environment_["PATH_INFO"] = extract_path_info(request.getPath(), location->getCgiExtension());
	environment_["PATH_TRANSLATED"] = location->getRootLocation() + 
		(environment_["PATH_INFO"].empty() ? "/" : environment_["PATH_INFO"]);
	environment_["QUERY_STRING"] = decode_url(request.getQuery());
	environment_["DOCUMENT_ROOT"] = location->getRootLocation();
	environment_["REQUEST_URI"] = request.getPath() + request.getQuery();
	build_environment_array();
	build_argument_array(interpreter_path);
}

void Cgi::initialize_interpreter_environment(Request& request, const std::vector<Location>::iterator location)
{
	std::string interpreter_path = "cgi/" + location->getCgiPath()[0];
	script_path_ = make_absolute_path(script_path_);
	set_content_variables(request);
	set_common_environment_vars(interpreter_path);
	set_server_variables(request);
	set_request_headers(request);
	environment_["REQUEST_URI"] = request.getPath() + request.getQuery();
	environment_["PATH_INFO"] = extract_path_info(request.getPath(), location->getCgiExtension());
	build_environment_array();
	build_argument_array(interpreter_path);
}

void Cgi::execute(short& error_code)
{
	if (!arg_array_ || !arg_array_[0] || !arg_array_[1] || !env_array_)
	{
		PrintApp::printEvent(RED_BOLD, SUCCESS, "CGI execute failed: Invalid arguments.");
		error_code = 500;
		return;
	}
	if (pipe(pipe_in) < 0 || pipe(pipe_out) < 0)
	{
		PrintApp::printEvent(RED_BOLD, SUCCESS, "Failed to create pipes for CGI: %s.", strerror(errno));
		close(pipe_in[0]); close(pipe_in[1]);
		close(pipe_out[0]); close(pipe_out[1]);
		error_code = 500;
		return;
	}
	process_id_ = fork();
	if (process_id_ == 0)
	{
		dup2(pipe_in[0], STDIN_FILENO);
		dup2(pipe_out[1], STDOUT_FILENO);
		close(pipe_in[0]); close(pipe_in[1]);
		close(pipe_out[0]); close(pipe_out[1]);
		execve(arg_array_[0], arg_array_, env_array_);
		std::string error_msg = "execve failed: " + std::string(strerror(errno)) + "\n";
		write(STDOUT_FILENO, error_msg.c_str(), error_msg.length());
		exit(1);
	}
	else if (process_id_ < 0)
	{
		PrintApp::printEvent(RED_BOLD, SUCCESS, "Fork failed: %s.", strerror(errno));
		close(pipe_in[0]); close(pipe_in[1]);
		close(pipe_out[0]); close(pipe_out[1]);
		error_code = 500;
	}
	else
	{
		close(pipe_in[0]);
		close(pipe_out[1]);
	}
}

bool Cgi::is_file_upload() const
{
	std::map<std::string, std::string>::const_iterator it = environment_.find("CONTENT_TYPE");
	return (it != environment_.end() && it->second.find("multipart/form-data") != std::string::npos);
}

std::string Cgi::decode_url(const std::string& encoded) const
{
	std::string result = encoded;
	size_t pos = result.find('%');
	while (pos != std::string::npos)
	{
		if (pos + 2 >= result.length())
			break;
		char decoded_char = static_cast<char>(hex_to_decimal(result.substr(pos + 1, 2)));
		if (decoded_char == 0 && result.substr(pos + 1, 2) != "00")
			break;
		result.replace(pos, 3, 1, decoded_char);
		pos = result.find('%', pos + 1);
	}
	return result;
}

unsigned int Cgi::hex_to_decimal(const std::string& hex) const
{
	unsigned int result = 0;
	for (size_t i = 0; i < hex.length(); ++i)
	{
		char c = hex[i];
		if (!std::isxdigit(c))
			return 0;
		result *= 16;
		if (c >= '0' && c <= '9')
			result += c - '0';
		else if (c >= 'A' && c <= 'F')
			result += c - 'A' + 10;
		else if (c >= 'a' && c <= 'f')
			result += c - 'a' + 10;
	}
	return result;
}

int Cgi::find_delimiter(const std::string& str, const std::string& delim) const
{
	if (str.empty())
		return -1;
	return static_cast<int>(str.find(delim));
}

std::string Cgi::make_absolute_path(const std::string& relative_path) const
{
	if (relative_path.empty() || relative_path[0] == '/')
		return relative_path;
	char* cwd = getcwd(NULL, 0);
	if (!cwd)
		return relative_path;
	std::string absolute = std::string(cwd) + "/" + relative_path;
	free(cwd);
	return absolute;
}

void Cgi::clear_resources()
{
	if (env_array_)
	{
		for (int i = 0; env_array_[i]; ++i)
			delete[] env_array_[i];
		delete[] env_array_;
		env_array_ = NULL;
	}
	if (arg_array_)
	{
		delete[] arg_array_[0];
		delete[] arg_array_[1];
		delete[] arg_array_;
		arg_array_ = NULL;
	}
	environment_.clear();
	script_path_.clear();
	process_id_ = -1;
	exit_status_ = 0;
}
