#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

int main()
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa))
	{
		std::cout << "wsa start up error";
		return 1;
	}

	SOCKET ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == INVALID_SOCKET)
	{
		std::cout << "socket invalid";
		return 1;
	}

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = INADDR_ANY;
	addr.sin_port = htons(5000);

	if (bind(ListenSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		closesocket(ListenSocket);
		WSACleanup();

		return 1;
	}

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		std::cerr << "listen failed\n";
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	std::cout << "Server listening on port 5000...\n";

	SOCKET ClientSocket = accept(ListenSocket, nullptr, nullptr);
	if (ClientSocket == INVALID_SOCKET) {
		std::cerr << "accept failed\n";
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	std::cout << "Client connected!\n";

	char recvbuf[512];
	int bytesReceived = recv(ClientSocket, recvbuf, sizeof(recvbuf), 0);
	if (bytesReceived > 0) {
		std::cout << "Received: " << std::string(recvbuf, bytesReceived) << "\n";
	}
	else {
		std::cerr << "recv failed\n";
	}

	closesocket(ClientSocket);
	closesocket(ListenSocket);
	WSACleanup();
}