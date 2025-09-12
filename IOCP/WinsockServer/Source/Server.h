#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

class Server;

enum ESocketOperation
{
	Accept = 0,
	Recv = 1,
	Send = 2,
};

struct SocketContext
{
	Server* MyServer;
	ESocketOperation LastOp;
	SOCKET Socket;
	OVERLAPPED* Overlapped;
};

class Server
{
public:
	Server();
	~Server();
	void Init();
	bool Start(long, short);
	void Stop();

protected:
	WSADATA WSAData;
	HANDLE CompletionPort;
	sockaddr_in ServerAddr;
	int PortNum;
	SOCKET ListenSocket;
	SOCKET AcceptSocket;
	char* AcceptAddrContext;

	LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs;

	bool CreateWorkerThreads();
	static DWORD WINAPI WorkerThread(LPVOID);
	void ProcessAccept(SocketContext* context);
	void ProcessReceive();
	void ProcessSend();
	void ProcessDisconnect();
};