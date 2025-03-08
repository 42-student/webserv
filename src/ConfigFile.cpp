#include "../inc/ConfigFile.hpp"

ConfigFile::ConfigFile()
{
	this->path = "";
}

ConfigFile::ConfigFile(std::string const path)
{
	this->path = path;
}

ConfigFile::~ConfigFile() {}

int ConfigFile::CheckFile(std::string const path, std::string const index)
{
	std::string full_path = path + index;
	if (getTypePath(index) == FILE_TYPE && checkAccessFile(index, R_OK) == 0)
		return 0;
	else if (getTypePath(full_path) == FILE_TYPE && checkAccessFile(full_path, R_OK) == 0)
		return 0;
	return -1;
}

int ConfigFile::getTypePath(std::string const path)
{
	struct stat buffer;
	int result = stat(path.c_str(), &buffer);
	if (result != 0)
		return INVALID_TYPE;
	else if (buffer.st_mode & S_IFREG)
		return FILE_TYPE;
	else if (buffer.st_mode & S_IFDIR)
		return DIRECTORY_TYPE;
	else
		return OTHER_TYPE;
}

int ConfigFile::checkAccessFile(std::string const path, int mode)
{
	return access(path.c_str(), mode);
}

std::string ConfigFile::readFile(std::string path)
{
	if (path.empty())
		return "";
	std::ifstream config_file(path.c_str());
	if (!config_file.is_open())
		return "";
	std::stringstream stream_binding;
	stream_binding << config_file.rdbuf();
	return stream_binding.str();
}
