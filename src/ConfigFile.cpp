#include "../inc/ConfigFile.hpp"

ConfigFile::ConfigFile() {}

ConfigFile::ConfigFile(std::string const path) : path(path) {}

ConfigFile::~ConfigFile() {}

int ConfigFile::CheckFile(std::string const path, std::string const index)
{
	if (getTypePath(index) == 1 && checkAccessFile(index, R_OK) == 0)
		return (0);
	if (getTypePath(path + index) == 1 && checkAccessFile(path + index, R_OK) == 0)
		return (0);
	return (-1);
}

int ConfigFile::getTypePath(std::string const path)
{
	struct stat buffer;
	int result;
	result = stat(path.c_str(), &buffer);
	if (result != 0)
		return (INVALID_TYPE);
	if (buffer.st_mode & S_IFREG)
		return (FILE_TYPE);
	if (buffer.st_mode & S_IFDIR)
		return (DIRECTORY_TYPE);
	return (OTHER_TYPE);
}

int ConfigFile::checkAccessFile(std::string const path, int mode)
{
	return (access(path.c_str(), mode));
}

std::string ConfigFile::readFile(std::string path)
{
	if (path.empty() || path.length() == 0)
		return (NULL);
	std::ifstream config_file(path.c_str());
	if (!config_file || !config_file.is_open())
		return (NULL);	
	std::stringstream stream_binding;
	stream_binding << config_file.rdbuf();
	return (stream_binding.str());
}
