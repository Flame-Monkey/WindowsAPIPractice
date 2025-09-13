#include "Server.h"

#include <iostream>

int main()
{
	Server server{};
	server.Init();
	server.Start(INADDR_LOOPBACK, 5000);

	while (true) {}
}