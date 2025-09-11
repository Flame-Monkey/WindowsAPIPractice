#include "Server.h"

#include <MSWSock.h>
#include <iostream>
#include <chrono>

HANDLE Server::CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

Server::Server()
{

}

Server::~Server()
{

}

// Assignment members in heap
void Server::Init()
{
}

// Listening Start
bool Server::Start(long ipv4Addr, short port)
{
	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		std::cerr << "WSAStartup() Error: " << WSAGetLastError() << std::endl;
		return false;
	}

	CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
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

	SocketContext* context = new SocketContext;
	context->LastOp = ESocketOperation::Accept;
	if (CreateIoCompletionPort((HANDLE)listenSocket, CompletionPort, (ULONG_PTR)context, 0) == NULL)
	{
		return false;
	}

	// !!
	GUID guidAcceptEx = WSAID_ACCEPTEX;
	LPFN_ACCEPTEX lpfnAcceptEx = nullptr;
	DWORD bytes = 0;
	if (SOCKET_ERROR == WSAIoctl(
		listenSocket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidAcceptEx,
		sizeof(guidAcceptEx),
		&lpfnAcceptEx,
		sizeof(lpfnAcceptEx),
		&bytes,
		NULL,
		NULL))
	{
		std::cerr << "WSAIoctl(AcceptEx) Error: " << WSAGetLastError() << std::endl;
		return false;
	}
	SOCKET acceptSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	char acceptBuffer[(sizeof(SOCKADDR_IN) + 16) * 2];
	DWORD bytesReceived = 0;

	OVERLAPPED overlapped = { 0 };
	if (!lpfnAcceptEx(
		listenSocket,
		acceptSocket,
		acceptBuffer,
		0,
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		&bytesReceived,
		&overlapped))
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			std::cerr << "AcceptEx Error: " << WSAGetLastError() << std::endl;
			closesocket(acceptSocket);
			return false;
		}
	}

	std::cout << "Server Listening On port " << port << std::endl;
	return true;
}

void Server::Stop()
{

}

bool Server::CreateWorkerThreads()
{
	HANDLE hThread = INVALID_HANDLE_VALUE;

	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&Server::WorkerThread, this, 0, NULL);

	return true;
}

DWORD WINAPI Server::WorkerThread(LPVOID context)
{
	int byteTransferred;
	SocketContext* sockCont = nullptr;
	LPWSAOVERLAPPED lpOverlapped = NULL;
	while (true)
	{
		GetQueuedCompletionStatus((HANDLE)CompletionPort, (LPDWORD)&byteTransferred, (PULONG_PTR)&sockCont, &lpOverlapped, INFINITE);
		switch (sockCont->LastOp)
		{
		case ESocketOperation::Accept:
			ProcessAccept();
			break;
		}
	}
}

void Server::ProcessAccept()
{
	std::cout << "ProcessACcept Called" << std::endl;


	return;
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