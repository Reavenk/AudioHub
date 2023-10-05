#include <iostream>
#include "Server.h"

int wsport = 8080;

Server server;

int main()
{
	{
		ServerLockedGuards lockGuard(&server);
		server.InitializeServer(wsport, lockGuard);
		std::cout << "Server started on port " << wsport << std::endl;
	}

	while (server.ConnectionState() != ConState::Closed)
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	
	return 0;
}