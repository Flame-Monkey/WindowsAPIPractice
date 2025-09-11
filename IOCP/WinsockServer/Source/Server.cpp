#include "Server.h"

#include <MSWSock.h>
#include <iostream>

Server::Server()
{

}

Server::~Server()
{

}

// Assignment members in heap
void Server::Init()
{
	HANDLE CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
}

// Listening Start
bool Server::Start(long ipv4Addr, short port)
{
	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		std::cerr << "WSAStartup() Error: " << WSAGetLastError() << std::endl;
		return false;
	}

	if (!CreateWorkerThreads())
	{
		return false;
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = htonl(ipv4Addr);
	serverAddr.sin_port = htons(port);

	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET)
	{
		std::cerr << "socket() Error: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return false;
	}

	if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		return false;
	}

	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		return false;
	}

	return true;
}

void Server::Stop()
{

}

bool Server::CreateWorkerThreads()
{
	HANDLE hThread = INVALID_HANDLE_VALUE;
}

void Server::WorkerThread()
{

}

void Server::ProcessAccept()
{

}

void Server::ProcessReceive()
{

}

void Server::ProcessSend()
{

}

void Server::ProcessDisconnect()
{

}