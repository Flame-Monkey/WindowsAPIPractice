#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

enum ESocketOperation
{
	Accept,
	Recv,
	Send,
};

struct SocketContext
{
	ESocketOperation LastOp;
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
	WSADATA wsaData;
	static HANDLE CompletionPort;
	SOCKET listenSocket;
	SOCKET acceptSocket;
	sockaddr_in serverAddr;
	int portNum;

	bool CreateWorkerThreads();
	static DWORD WINAPI WorkerThread(LPVOID);
	static void ProcessAccept();
	void ProcessReceive();
	void ProcessSend();
	void ProcessDisconnect();
};