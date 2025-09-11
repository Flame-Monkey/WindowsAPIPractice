#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

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
	SOCKET listenSocket;
	sockaddr_in serverAddr;
	int portNum;

	bool CreateWorkerThreads();
	void WorkerThread();
	void ProcessAccept();
	void ProcessReceive();
	void ProcessSend();
	void ProcessDisconnect();
};