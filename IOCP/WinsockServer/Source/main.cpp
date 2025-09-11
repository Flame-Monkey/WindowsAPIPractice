#include "Server.h"

#include <iostream>

int main()
{
	int i = ESocketOperation::Accept;
	std::cout << i << std::endl;

	Server server;
	server.Start(INADDR_LOOPBACK, 5000);

	while (true) {}
}