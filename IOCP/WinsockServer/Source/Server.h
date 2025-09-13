#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <list>

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
	OVERLAPPED Overlapped;
	Server* MyServer;
	ESocketOperation LastOp;
	SOCKET Socket;
	WSABUF* DataBuf;
	int ImmediatelyReceivedBytes; // Won't use WSAGetOverlappedResult
	int Flags;
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
	SocketContext* AcceptContext;
	std::list<SOCKET>* ConnectedSockets;

	static LPFN_ACCEPTEX lpfnAcceptEx;
	static LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs;

	bool InitAcceptEx(SOCKET);

	bool CreateWorkerThreads();
	static DWORD WINAPI WorkerThread(LPVOID);
	void ProcessAccept(SocketContext* context);
	void AcceptClient();
	void ProcessReceive(SocketContext*, int);
	void ProcessSend();
	void BroadCast(char* buffer, int length);
	void ProcessDisconnect();
};