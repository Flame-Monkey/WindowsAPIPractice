#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

int main()
{
	WSADATA WsaData;
	if (WSAStartup(MAKEWORD(2, 2), &WsaData))
	{
		return 1;
	}

	HANDLE handler;
	if (!(handler = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)))
	{
		return 1;
	}


}