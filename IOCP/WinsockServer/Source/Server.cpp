#include "Server.h"

#include <iostream>
#include <chrono>

LPFN_ACCEPTEX Server::lpfnAcceptEx = nullptr;
LPFN_GETACCEPTEXSOCKADDRS Server::lpfnGetAcceptExSockaddrs = nullptr;

Server::Server() :
	WSAData{ 0 },
	CompletionPort{ INVALID_HANDLE_VALUE },
	ServerAddr{},
	PortNum{ 0 },
	ListenSocket{ INVALID_SOCKET },
	AcceptSocket{ INVALID_SOCKET },
	AcceptContext{ nullptr }
{

}

Server::~Server()
{

}

// Assignment members in heap
void Server::Init()
{
	AcceptContext = new SocketContext;
	AcceptContext->MyServer = this;
	AcceptContext->LastOp = ESocketOperation::Accept;
	AcceptContext->DataBuf = new WSABUF;
	AcceptContext->DataBuf->buf = new char[(sizeof(SOCKADDR_IN) + 16) * 2];
	ZeroMemory(AcceptContext->DataBuf->buf, (sizeof(SOCKADDR_IN) + 16) * 2);
	AcceptContext->DataBuf->len = (sizeof(SOCKADDR_IN) + 16) * 2;
	ZeroMemory(&AcceptContext->Overlapped, sizeof(OVERLAPPED));

	ConnectedSockets = new std::list<SOCKET>{};
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

	if (CreateIoCompletionPort((HANDLE)ListenSocket, CompletionPort, (ULONG_PTR)AcceptContext, 0) == NULL)
	{
		return false;
	}

	AcceptSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (AcceptSocket == INVALID_SOCKET)
	{
		std::cerr << "Accept Socket Error: " << WSAGetLastError() << std::endl;
		return false;
	}

	if (!InitAcceptEx(ListenSocket))
	{
		return false;
	}
	if (!lpfnAcceptEx(ListenSocket, AcceptSocket, AcceptContext->DataBuf->buf,
		0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, NULL, &AcceptContext->Overlapped))
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

bool Server::InitAcceptEx(SOCKET listenSocket)
{
	if (lpfnAcceptEx == nullptr)
	{
		GUID guidAcceptEx = WSAID_ACCEPTEX;
		DWORD bytes = 0;

		if (WSAIoctl(listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidAcceptEx, sizeof(guidAcceptEx),
			&lpfnAcceptEx, sizeof(lpfnAcceptEx), &bytes, NULL, NULL) == SOCKET_ERROR)
		{
			std::cerr << "WSAIoctl(AcceptEx) Error: " << WSAGetLastError() << std::endl;
			return false;
		}
	}


	if (lpfnGetAcceptExSockaddrs == nullptr)
	{
		GUID guidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
		DWORD dwBytes = 0;

		if(WSAIoctl(listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidGetAcceptExSockaddrs, sizeof(guidGetAcceptExSockaddrs),
			&lpfnGetAcceptExSockaddrs, sizeof(lpfnGetAcceptExSockaddrs), &dwBytes, NULL, NULL) == SOCKET_ERROR) 
		{
			printf("WSAIoctl for GetAcceptExSockaddrs failed: %d\n", WSAGetLastError());
			return false;
		}
	}

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
		std::cout << "Transferred: " << byteTransferred << std::endl;
		SocketContext* s = (SocketContext*) lpOverlapped;
		std::cout << s->LastOp << std::endl;
		std::cout << "Test " << (sockCont == (void*)lpOverlapped) << std::endl;
		switch (s->LastOp)
		{
		case ESocketOperation::Accept:
			std::cout << "Process Accept\n";
			server->ProcessAccept(sockCont);
			break;
		case ESocketOperation::Recv:
			std::cout << "Process receive\n";
			server->ProcessReceive(sockCont, byteTransferred);
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
	lpfnGetAcceptExSockaddrs(
		AcceptContext->DataBuf->buf,
		0,
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		&localAddr,
		&localLen,
		&remoteAddr,
		&remoteLen
	);

	char ipstring[32];
	std::cout << "Client Connected. IPAddress: "
		<< inet_ntop(AF_INET, &((sockaddr_in*)remoteAddr)->sin_addr, ipstring, 20) 
		<< " Port: " << ntohs(((SOCKADDR_IN*)remoteAddr)->sin_port) << std::endl;

	AcceptClient();
	ZeroMemory(AcceptContext->DataBuf->buf, AcceptContext->DataBuf->len);
	ZeroMemory(&AcceptContext->Overlapped, sizeof(OVERLAPPED));
	AcceptSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (AcceptSocket == INVALID_SOCKET)
	{
		std::cerr << "Accept Socket Error: " << WSAGetLastError() << std::endl;
		return;
	}
	if (!lpfnAcceptEx(ListenSocket, AcceptSocket, AcceptContext->DataBuf->buf,
		0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, NULL, &AcceptContext->Overlapped))
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			std::cerr << "AcceptEx Error: " << WSAGetLastError() << std::endl;
			closesocket(AcceptSocket);
			return;
		}
	}

	std::cout << "New accept start\n";

	return;
}

void Server::AcceptClient()
{
	if (AcceptSocket == INVALID_SOCKET)
	{
		std::cerr << "Accept Socket Invalid" << std::endl;
		return;
	}
	ConnectedSockets->push_back(AcceptSocket);

	SocketContext* context = new SocketContext;
	context->MyServer = this;
	context->Socket = AcceptSocket;
	context->LastOp = ESocketOperation::Recv;
	context->DataBuf = new WSABUF;
	context->DataBuf->buf = new char[2048];
	context->DataBuf->len = 2048;
	context->Flags = 0;
	ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));

	if (CreateIoCompletionPort((HANDLE)AcceptSocket, CompletionPort, (ULONG_PTR)context, 0) == NULL)
	{
		std::cerr << "CreateIoCompletionPort() Error: " << WSAGetLastError() << std::endl;
		return;
	}

	if (WSARecv(AcceptSocket, context->DataBuf, 1, (LPDWORD)&context->ImmediatelyReceivedBytes,
		(LPDWORD) & context->Flags, &context->Overlapped, NULL) == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING)
	{
		std::cerr << "WSARecv() Error: " << WSAGetLastError() << std::endl;
		return;
	}
	std::cout << "Recv Started\n";


	return;
}

void Server::ProcessReceive(SocketContext* context, int bytesTransferred)
{
	if (bytesTransferred == 0)
	{
		std::cout << "Client disconnected." << std::endl;
		std::cout << "test n\n";
		closesocket(context->Socket);
		std::cout << "test nn\n";
		delete context;
		std::cout << "test nnn\n";
		return;
	}
	context->DataBuf->buf[bytesTransferred] = '\0';

	std::cout << "Received: " << context->DataBuf->buf << std::endl;

	BroadCast(context->DataBuf->buf, bytesTransferred);

	ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));
	context->LastOp = ESocketOperation::Recv;
	context->ImmediatelyReceivedBytes = 0;
	context->Flags = 0;

	DWORD recvBytes = 0;
	if (WSARecv(context->Socket, context->DataBuf, 1, &recvBytes,
		(LPDWORD)&context->Flags, &context->Overlapped, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			std::cerr << "WSARecv Error: " << WSAGetLastError() << std::endl;
			closesocket(context->Socket);
			delete context;
		}
	}
}


void Server::ProcessSend()
{

}

void Server::BroadCast(char* buffer, int length)
{
	std::cout << "BroadCast!!!\n";

	for(auto s : *ConnectedSockets)
	{
		std::cout << "BroadCast...\n";
		SocketContext* context = new SocketContext;
		context->LastOp = ESocketOperation::Send;
		context->DataBuf = new WSABUF;
		context->DataBuf->len = length;
		context->DataBuf->buf = new char[length];
		std::copy(buffer, buffer + length, context->DataBuf->buf);
		context->MyServer = this;
		context->Socket = s;
		context->Flags = 0;
		ZeroMemory(&context->Overlapped, sizeof(OVERLAPPED));

		if (WSASend(context->Socket, context->DataBuf, 1, &context->DataBuf->len, context->Flags,
			&context->Overlapped, NULL) == SOCKET_ERROR)
		{
			std::cerr << "WSASend Error: " << WSAGetLastError() << std::endl;
			closesocket(context->Socket);
			delete context;
		}
	}
}

void Server::ProcessDisconnect()
{

}