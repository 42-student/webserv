#include "../inc/Response.hpp"

Response::Response() 
	: target_file_(""), body_content_(""), redirect_loc_(""), body_size_(0), 
	cgi_flag_(0), cgi_response_size_(0), auto_index_(false), status_code_(0), full_response_("")
{
	body_data_.clear();
	setupMimeTypes();
}

Response::Response(Request& req) 
	: target_file_(""), body_content_(""), redirect_loc_(""), body_size_(0), cgi_flag_(0), 
	cgi_response_size_(0), auto_index_(false), status_code_(0), req_(req), full_response_("")
{
	body_data_.clear();
	setupMimeTypes();
}

Response::~Response() {}

void Response::setupMimeTypes()
{
	mime_types_["default"] = "text/html";
	mime_types_[".html"] = "text/html";
	mime_types_[".jpg"] = "image/jpg";
	mime_types_[".jpeg"] = "image/jpeg";
	mime_types_[".png"] = "image/png";
	mime_types_[".ico"] = "image/x-icon";
	mime_types_[".svg"] = "image/svg+xml";
	mime_types_[".bmp"] = "image/bmp";
}

void Response::constructResponse()
{
	if (hasRequestError() || prepareBody())
		generateErrorPage();
	if (cgi_flag_)
		return;
	if (auto_index_)
	{
		if (createDirListing(target_file_, body_data_, body_size_))
		{
			status_code_ = 500;
			generateErrorPage();
		} else
		{
			status_code_ = 200;
			body_content_.assign(body_data_.begin(), body_data_.end());
		}
	}
	writeStatus();
	writeHeaders();
	if (req_.getHttpMethod() == GET || status_code_ != 200)
		full_response_ += body_content_;
}

bool Response::hasRequestError()
{
	if (req_.errorCodes())
	{
		status_code_ = req_.errorCodes();
		return true;
	}
	return false;
}

int Response::prepareBody()
{
	if (exceedsBodyLimit())
	{
		status_code_ = 413;
		return 1;
	}
	if (processTarget())
		return 1;
	if (status_code_)
		return 0;
	status_code_ = 200;
	int method = req_.getHttpMethod();
	if (method == GET)
		return doGet();
	else if (method == POST)
		return doPost();
	else if (method == DELETE)
		return doDelete();
	else
	{
		status_code_ = 501;
		return 1;
	}
	return 0;
}

void Response::generateErrorPage()
{
	if (!server_.getErrorPages().count(status_code_) || server_.getErrorPages().at(status_code_).empty() || 
			req_.getHttpMethod() == DELETE || req_.getHttpMethod() == POST)
		setDefaultError();
	else
	{
		if (status_code_ >= 400 && status_code_ < 500)
		{
			redirect_loc_ = server_.getErrorPages().at(status_code_);
			if (!redirect_loc_.empty() && redirect_loc_[0] != '/')
				redirect_loc_ = "/" + redirect_loc_;
			status_code_ = 302;
		}
		target_file_ = server_.getRoot() + server_.getErrorPages().at(status_code_);
		short prev_code = status_code_;
		if (loadFile())
		{
			status_code_ = prev_code;
			body_content_ = makeErrorPage(status_code_);
		}
	}
}

int Response::createDirListing(const std::string& dir, std::vector<unsigned char>& data, size_t& size)
{
	DIR* dir_handle = opendir(dir.c_str());
	if (!dir_handle)
	{
		std::cerr << "Failed to open directory" << std::endl;
		return 1;
	}
	std::string listing = "<html>\n<head>\n<title>Index of " + dir + "</title>\n</head>\n"
						 "<body>\n<h1>Index of " + dir + "</h1>\n"
						 "<table style=\"width:80%; font-size:15px\">\n<hr>\n"
						 "<th style=\"text-align:left\">File Name</th>\n"
						 "<th style=\"text-align:left\">Last Modified</th>\n"
						 "<th style=\"text-align:left\">Size</th>\n";
	struct dirent* entry;
	struct stat info;
	while ((entry = readdir(dir_handle)))
	{
		if (strcmp(entry->d_name, ".") == 0)
			continue;
		std::string path = dir + entry->d_name;
		if (stat(path.c_str(), &info) != 0)
			continue;
		listing += "<tr>\n<td><a href=\"" + std::string(entry->d_name) + (S_ISDIR(info.st_mode) ? "/" : "") + "\">" + 
					entry->d_name + (S_ISDIR(info.st_mode) ? "/" : "") + "</a></td>\n<td>" + 
					ctime(&info.st_mtime) + "</td>\n<td>";
		if (!S_ISDIR(info.st_mode))
		{
			std::ostringstream ss;
			ss << info.st_size;
			listing += ss.str();
		}
		listing += "</td>\n</tr>\n";
	}
	listing += "</table>\n<hr>\n</body>\n</html>\n";
	data.assign(listing.begin(), listing.end());
	size = data.size();
	closedir(dir_handle);
	return 0;
}

bool Response::exceedsBodyLimit()
{
	return req_.getBody().length() > server_.getClientMaxBodySize();
}

int Response::processTarget()
{
	std::string loc_key;
	findLocation(req_.getPath(), server_.getLocations(), loc_key);
	if (!loc_key.empty())
	{
		Location loc = *server_.getLocationKey(loc_key);
		if (methodDisallowed(loc))
		{
			std::cout << "Method not allowed\n";
			return 1;
		}
		if (bodyTooLarge(req_.getBody(), loc))
		{
			status_code_ = 413;
			return 1;
		}
		if (hasReturn(loc) || hasRedirect(loc))
			return 1;
		if (isCgi(loc.getPath()))
			return manageCgi(loc_key);
		if (!loc.getAlias().empty())
			applyAlias(loc, req_, target_file_);
		else
			addRoot(loc, req_, target_file_);
		if (!loc.getCgiExtension().empty() && matchesCgiExt(target_file_, loc))
			return handleTempCgi(loc_key);
		if (isDir(target_file_))
		{
			if (target_file_[target_file_.length() - 1] != '/')
			{
				status_code_ = 301;
				redirect_loc_ = req_.getPath() + "/";
				return 1;
			}
			return processIndex(loc.getIndexLocation(), loc.getAutoindex());
		}
	}
	else
		return handleNoLocation(server_.getRoot(), req_);
	return 0;
}

bool Response::methodDisallowed(Location& loc)
{
	return checkMethod(req_.getHttpMethod(), loc, status_code_);
}

bool Response::checkMethod(int method, Location& loc, short& code)
{
	std::vector<short> allowed = loc.getMethods();
	if ((method == 0 && !allowed[0]) || (method == 1 && !allowed[1]) || (method == 2 && !allowed[2]))
	{
		code = 405;
		return true;
	}
	return false;
}

bool Response::bodyTooLarge(const std::string& body, const Location& loc)
{
	return body.length() > loc.getMaxBodySize();
}

bool Response::hasReturn(Location& loc)
{
	return evalReturn(loc, status_code_, redirect_loc_);
}

bool Response::evalReturn(Location& loc, short& code, std::string& redirect)
{
	if (loc.getReturn().empty())
		return false;
	code = 301;
	redirect = loc.getReturn();
	if (!redirect.empty() && redirect[0] != '/')
		redirect = "/" + redirect;
	return true;
}

bool Response::hasRedirect(Location& loc)
{
	return evalRedirect(loc, status_code_, redirect_loc_);
}

bool Response::evalRedirect(Location& loc, short& code, std::string& redirect)
{
	if (loc.getRedirect().empty())
		return false;
	code = 301;
	redirect = loc.getRedirect();
	return true;
}

bool Response::isCgi(const std::string& path)
{
	return path.find("cgi") != std::string::npos;
}

int Response::manageCgi(std::string& loc_key)
{
	std::string path = req_.getPath();
	size_t pos = 0;
	if (!validatePath(path, loc_key, pos) || !validExt(path) || !validFile(path) || !filePermitted(req_, loc_key))
		return 1;
	return initCgi(path, loc_key) ? 1 : 0;
}

bool Response::validatePath(std::string& path, std::string& loc_key, size_t& pos)
{
	if (!path.empty() && path[0] == '/')
		path.erase(0, 1);
	if (path == "cgi")
		path += "/" + server_.getLocationKey(loc_key)->getIndexLocation();
	else if (path == "cgi/")
		path += server_.getLocationKey(loc_key)->getIndexLocation();
	pos = path.find(".");
	if (pos == std::string::npos)
	{
		status_code_ = 501;
		return false;
	}
	return true;
}

bool Response::validExt(const std::string& path)
{
	if (path.substr(path.find(".")) != ".py")
	{
		status_code_ = 501;
		return false;
	}
	return true;
}

bool Response::validFile(const std::string& path)
{
	if (ConfigFile::getTypePath(path) != 1)
	{
		status_code_ = 404;
		return false;
	}
	if (ConfigFile::checkAccessFile(path, 1) == -1 || ConfigFile::checkAccessFile(path, 3) == -1)
	{
		status_code_ = 403;
		return false;
	}
	return true;
}

bool Response::filePermitted(Request& req, std::string& loc_key)
{
	return !checkMethod(req.getHttpMethod(), *server_.getLocationKey(loc_key), status_code_);
}

bool Response::initCgi(const std::string& path, std::string& loc_key)
{
	cgi_handler_.clear();
	cgi_handler_.setCgiPath(path);
	cgi_flag_ = 1;
	if (pipe(cgi_pipe_) < 0)
	{
		status_code_ = 500;
		return true;
	}
	cgi_handler_.initEnv(req_, server_.getLocationKey(loc_key));
	cgi_handler_.execute(status_code_);
	return false;
}

void Response::applyAlias(Location& loc, Request& req, std::string& target)
{
	target = joinPaths(loc.getAlias(), req.getPath().substr(loc.getPath().length()), "");
}

void Response::addRoot(Location& loc, Request& req, std::string& target)
{
	target = joinPaths(loc.getRootLocation(), req.getPath(), "");
}

std::string Response::joinPaths(const std::string& p1, const std::string& p2, const std::string& p3)
{
	std::string result = p1;
	if (!result.empty() && result[result.length() - 1] == '/' && !p2.empty() && p2[0] == '/')
		result += p2.substr(1);
	else if (!result.empty() && result[result.length() - 1] != '/' && !p2.empty() && p2[0] != '/')
		result += "/" + p2;
	else
		result += p2;
	if (!result.empty() && result[result.length() - 1] == '/' && !p3.empty() && p3[0] == '/')
		result += p3.substr(1);
	else if (!result.empty() && result[result.length() - 1] != '/' && !p3.empty() && p3[0] != '/')
		result += "/" + p3;
	else
		result += p3;
	return result;
}

int Response::handleTempCgi(std::string& loc_key)
{
	cgi_handler_.clear();
	cgi_handler_.setCgiPath(target_file_);
	cgi_flag_ = 1;
	if (pipe(cgi_pipe_) < 0)
	{
		status_code_ = 500;
		return 1;
	}
	cgi_handler_.initEnvCgi(req_, server_.getLocationKey(loc_key));
	cgi_handler_.execute(status_code_);
	return 0;
}

bool Response::isDir(const std::string& path)
{
	struct stat st;
	return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

bool Response::processIndex(const std::string& index, bool autoindex)
{
	target_file_ += index.empty() ? server_.getIndex() : index;
	if (!exists(target_file_))
	{
		if (autoindex)
		{
			target_file_.erase(target_file_.rfind('/') + 1);
			auto_index_ = true;
			return false;
		}
		status_code_ = 403;
		return true;
	}
	if (isDir(target_file_))
	{
		status_code_ = 301;
		redirect_loc_ = joinPaths(req_.getPath(), index.empty() ? server_.getIndex() : index, "");
		if (!redirect_loc_.empty() && redirect_loc_[redirect_loc_.length() - 1] != '/')
			redirect_loc_ += "/";
		return true;
	}
	return false;
}

bool Response::exists(const std::string& file)
{
	std::ifstream f(file.c_str());
	bool good = f.good();
	f.close();
	return good;
}

bool Response::handleNoLocation(const std::string& root, Request& req)
{
	target_file_ = joinPaths(root, req.getPath(), "");
	if (isDir(target_file_))
	{
		if (target_file_[target_file_.length() - 1] != '/')
		{
			status_code_ = 301;
			redirect_loc_ = req.getPath() + "/";
			return true;
		}
		target_file_ += server_.getIndex();
		if (!exists(target_file_))
		{
			status_code_ = 403;
			return true;
		}
		if (isDir(target_file_))
		{
			status_code_ = 301;
			redirect_loc_ = joinPaths(req.getPath(), server_.getIndex(), "");
			if (!redirect_loc_.empty() && redirect_loc_[redirect_loc_.length() - 1] != '/')
				redirect_loc_ += "/";
			return true;
		}
	}
	return false;
}

void Response::setRequest(Request& req)
{
	req_ = req;
}

void Response::setServer(Server& srv)
{
	server_ = srv;
}

void Response::writeStatus()
{
	std::ostringstream ss;
	ss << "HTTP/1.1 " << status_code_ << " " << Server::statusCodeString(status_code_) << "\r\n";
	full_response_ = ss.str();
}

void Response::writeHeaders()
{
	addContentType();
	addContentLength();
	addConnection();
	addServer();
	addLocation();
	addDate();
	full_response_ += "\r\n";
}

void Response::setDefaultError()
{
	body_content_ = makeErrorPage(status_code_);
}

void Response::setStatus(short code)
{
	status_code_ = code;
}

void Response::setError(short code)
{
	full_response_.clear();
	status_code_ = code;
	body_content_.clear();
	generateErrorPage();
	writeStatus();
	writeHeaders();
	full_response_ += body_content_;
}

void Response::toggleCgi(int state)
{
	cgi_flag_ = state;
}

void Response::findLocation(const std::string& path, const std::vector<Location>& locs, std::string& key)
{
	size_t longest = 0;
	for (std::vector<Location>::const_iterator it = locs.begin(); it != locs.end(); ++it)
	{
		if (path.find(it->getPath()) == 0 && 
			(it->getPath() == "/" || path.length() == it->getPath().length() || path[it->getPath().length()] == '/'))
		{
			if (it->getPath().length() > longest)
			{
				longest = it->getPath().length();
				key = it->getPath();
			}
		}
	}
}

std::string Response::getResponse()
{
	return full_response_;
}

size_t Response::getLength() const
{
	return full_response_.length();
}

int Response::getStatus() const
{
	return status_code_;
}

int Response::getCgiFlag()
{
	return cgi_flag_;
}

std::string Response::makeErrorPage(short code)
{
	std::ostringstream ss;
	ss << "<html>\r\n<head><title>" << code << " " << Server::statusCodeString(code) 
		<< "</title></head>\r\n<body>\r\n<center><h1>" << code << " " << Server::statusCodeString(code) 
		<< "</h1></center>\r\n";
	return ss.str();
}

std::string Response::fetchMimeType(const std::string& ext) const
{
	std::map<std::string, std::string>::const_iterator it = mime_types_.find(ext);
	return (it != mime_types_.end()) ? it->second : mime_types_.find("default")->second;
}

bool Response::knowsMimeType(const std::string& ext) const
{
	return mime_types_.find(ext) != mime_types_.end();
}

bool Response::isBoundary(const std::string& line, const std::string& boundary)
{
	return line == "--" + boundary + "\r";
}

bool Response::isEndBoundary(const std::string& line, const std::string& boundary)	{
	return line == "--" + boundary + "--\r";
}

bool Response::matchesCgiExt(const std::string& target, const Location& loc)
{
	return target.rfind(loc.getCgiExtension()[0]) != std::string::npos;
}

bool Response::skipBody()
{
	return cgi_flag_ || auto_index_;
}

bool Response::doGet()
{
	return loadFile() != 0;
}

bool Response::doPost()
{
	if (exists(target_file_) && req_.getHttpMethod() == POST)
	{
		status_code_ = 204;
		return false;
	}
	std::ofstream file(target_file_.c_str(), std::ios::binary);
	if (!file.is_open())
	{
		status_code_ = 404;
		return true;
	}
	std::string body = req_.getBody();
	if (req_.getMultiformFlag())
		body = stripBoundary(body, req_.getBoundary());
	file.write(body.c_str(), body.length());
	file.close();
	return false;
}

bool Response::doDelete()
{
	if (!exists(target_file_))
	{
		status_code_ = 404;
		return true;
	}
	if (remove(target_file_.c_str()) != 0)
	{
		status_code_ = 500;
		return true;
	}
	return false;
}

int Response::loadFile()
{
	std::ifstream file(target_file_.c_str());
	if (!file.is_open())
	{
		status_code_ = 404;
		return 1;
	}
	std::ostringstream ss;
	ss << file.rdbuf();
	body_content_ = ss.str();
	file.close();
	return 0;
}

void Response::addContentType()
{
	std::string::size_type pos = target_file_.rfind(".");
	std::string ext = (pos != std::string::npos) ? target_file_.substr(pos) : "";
	std::string mime = (pos != std::string::npos && status_code_ == 200) ? fetchMimeType(ext) : fetchMimeType("default");
	full_response_ += "Content-Type: " + mime + "\r\n";
}

void Response::addContentLength()
{
	std::ostringstream ss;
	ss << body_content_.length();
	full_response_ += "Content-Length: " + ss.str() + "\r\n";
}

void Response::addConnection()
{
	if (req_.getHeader("connection") == "keep-alive")
		full_response_ += "Connection: keep-alive\r\n";
}

void Response::addServer()
{
	full_response_ += "Server: miau\r\n";
}

void Response::addLocation()
{
	if (!redirect_loc_.empty())
		full_response_ += "Location: " + redirect_loc_ + "\r\n";
}

void Response::addDate()
{
	char buf[1000];
	time_t now = time(0);
	struct tm* tm = gmtime(&now);
	strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", tm);
	full_response_ += "Date: " + std::string(buf) + "\r\n";
}

std::string Response::stripBoundary(std::string& body, const std::string& boundary)
{
	std::string result;
	bool in_content = false;
	std::string::size_type pos = 0, last = 0;
	while ((pos = body.find("\n", pos)) != std::string::npos)
	{
		std::string line = body.substr(last, pos - last);
		if (isEndBoundary(line, boundary))
			break;
		if (isBoundary(line, boundary))
		{
			if (!getFilename(line).empty())
				in_content = true;
		}
		else if (in_content && line != "\r")
			result += line + "\n";
		last = ++pos;
	}
	body = result;
	return body;
}

std::string Response::getFilename(const std::string& line)
{
	std::string::size_type start = line.find("filename=\"");
	if (start == std::string::npos)
		return "";
	start += 10;
	std::string::size_type end = line.find("\"", start);
	return (end != std::string::npos) ? line.substr(start, end - start) : "";
}

void Response::trimResponse(size_t pos)
{
	full_response_ = full_response_.substr(pos);
}

void Response::reset()
{
	target_file_.clear();
	body_content_.clear();
	redirect_loc_.clear();
	body_data_.clear();
	body_size_ = 0;
	cgi_flag_ = 0;
	cgi_response_size_ = 0;
	auto_index_ = false;
	status_code_ = 0;
	full_response_.clear();
}
