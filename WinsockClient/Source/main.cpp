#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

int main()
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa))
	{
		return 1;
	}

	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
	{
		return 1;
	}

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(5000);
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

	connect(s, (sockaddr*)&addr, sizeof(addr));

	const char* msg = "hello, server!!";
	send(s, msg, (int)strlen(msg), 0);

	closesocket(s);
	WSACleanup();

	return 0;
}