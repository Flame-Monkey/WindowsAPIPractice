#include "Server.h"

#include <iostream>
#include <format>
#include <chrono>

Server::Server():
	WSAData{ 0 },
	CompletionPort{ INVALID_HANDLE_VALUE }
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
	if (WSAStartup(MAKEWORD(2, 2), &WSAData))
	{
		std::cerr << "WSAStartup() Error: " << WSAGetLastError() << std::endl;
		return false;
	}

	CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (!CreateWorkerThreads())
	{
		return false;
	}

	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_addr.S_un.S_addr = htonl(ipv4Addr);
	ServerAddr.sin_port = htons(port);

	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == INVALID_SOCKET)
	{
		std::cerr << "socket() Error: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return false;
	}

	if (bind(ListenSocket, (sockaddr*)&ServerAddr, sizeof(ServerAddr)) == SOCKET_ERROR)
	{
		return false;
	}

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		return false;
	}

	SocketContext* context = new SocketContext;
	context->MyServer = this;
	context->LastOp = ESocketOperation::Accept;
	context->Overlapped = new OVERLAPPED;
	ZeroMemory(context->Overlapped, sizeof(OVERLAPPED));

	if (CreateIoCompletionPort((HANDLE)ListenSocket, CompletionPort, (ULONG_PTR)context, 0) == NULL)
	{
		return false;
	}

	// load AcceptEx function pointer
	GUID guidAcceptEx = WSAID_ACCEPTEX;
	LPFN_ACCEPTEX lpfnAcceptEx = nullptr;
	DWORD bytes = 0;
	if (WSAIoctl(ListenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidAcceptEx, sizeof(guidAcceptEx),
		&lpfnAcceptEx, sizeof(lpfnAcceptEx), &bytes, NULL, NULL) == SOCKET_ERROR)
	{
		std::cerr << "WSAIoctl(AcceptEx) Error: " << WSAGetLastError() << std::endl;
		return false;
	}

	AcceptSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(AcceptSocket == INVALID_SOCKET)
	{
		std::cerr << "Accept Socket Error: " << WSAGetLastError() << std::endl;
		return false;
	}
	AcceptAddrContext = new char[(sizeof(SOCKADDR_IN) + 16) * 2];
	ZeroMemory(AcceptAddrContext, (sizeof(SOCKADDR_IN) + 16) * 2);
	if (!lpfnAcceptEx(ListenSocket, AcceptSocket, AcceptAddrContext,
		0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, NULL, context->Overlapped))
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			std::cerr << "AcceptEx Error: " << WSAGetLastError() << std::endl;
			closesocket(AcceptSocket);
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

DWORD WINAPI Server::WorkerThread(LPVOID serverInstance)
{
	Server* server = ((Server*)serverInstance);
	int byteTransferred;
	SocketContext* sockCont = nullptr;
	LPWSAOVERLAPPED lpOverlapped = NULL;

	while (true)
	{
		GetQueuedCompletionStatus(server->CompletionPort, (LPDWORD)&byteTransferred, (PULONG_PTR)&sockCont, &lpOverlapped, INFINITE);
		switch (sockCont->LastOp)
		{
		case ESocketOperation::Accept:
			server->ProcessAccept(sockCont);
			break;
		case ESocketOperation::Recv:
			server->ProcessReceive();
			break;
		case ESocketOperation::Send:
			server->ProcessSend();
			break;
		default:
			std::cout << "Unknown Operation" << std::endl;
		}
	}
}

void Server::ProcessAccept(SocketContext* context)
{
	sockaddr* localAddr = nullptr;
	sockaddr* remoteAddr = nullptr;
	int localLen = 0, remoteLen = 0;

	GUID guidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;

	DWORD dwBytes = 0;
	int result = WSAIoctl(
		ListenSocket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidGetAcceptExSockaddrs,
		sizeof(guidGetAcceptExSockaddrs),
		&lpfnGetAcceptExSockaddrs,
		sizeof(lpfnGetAcceptExSockaddrs),
		&dwBytes,
		NULL,
		NULL
	);

	if (result == SOCKET_ERROR) {
		printf("WSAIoctl for GetAcceptExSockaddrs failed: %d\n", WSAGetLastError());
		return;
	}

	lpfnGetAcceptExSockaddrs(
		AcceptAddrContext,
		0,
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		&localAddr,
		&localLen,
		&remoteAddr,
		&remoteLen
	);
	// lovalAddr, remoteAddr = nullptr;
	std::cout << localAddr << ' ' << remoteAddr << std::endl;
	
	for (int i = 0; i < (sizeof(sockaddr_in)); i++) 
	{ 
		printf("%d: %02d\n", i, *(((unsigned char*)localAddr) + i)); 
	}

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