#include "Server.h"

#include <iostream>

int main()
{
	sockaddr_in* addr = new sockaddr_in;
	ZeroMemory(addr, sizeof(sockaddr_in));
	addr->sin_family = AF_INET;
	addr->sin_addr.S_un.S_addr = htonl(INADDR_LOOPBACK);
	addr->sin_port = htons(5000);

	for (int i = 0; i < (sizeof(sockaddr_in)); i++)
	{
		printf("%d: %02d\n", i, *(((unsigned char*)addr) + i));
	}

	Server server;
	server.Start(INADDR_LOOPBACK, 5000);

	while (true) {}
}