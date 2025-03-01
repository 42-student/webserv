#include "../inc/Webserver.hpp"
#include "../inc/utils.hpp"

int main(int ac, char **av)
{
	try
	{
		if (ac != 2)
			throw std::invalid_argument("Invalid number of arguments. Usage: ./webserv <config_file>.conf.");
		signal(SIGPIPE, SIG_IGN);
		std::string configFile = (av[1]);
		Parser servParser;
		Webserver webServ;
		servParser.parsServConf(configFile);
		webServ.setupServers(servParser.getServers());
		webServ.processServerRequests();
	}
	catch (std::exception &err)
	{
		return oops(err.what());
	}
	return 0;
}
