## IOCP 

# Overlapped 방식 3개
Event, Callback, IOCP**

# 헤더 정보
<WinSock2.h> https://learn.microsoft.com/en-us/windows/win32/api/winsock2/
<WS2tcpip.h> https://learn.microsoft.com/en-us/windows/win32/api/ws2tcpip/

# 예제
https://github.com/microsoft/Windows-classic-samples/blob/main/Samples/Win7Samples/netds/winsock/iocp/server/IocpServer.Cpp

주요 구조 흐름
WSAStartup      WinSockAPI Startup
-> CreateIoCompletionPort -> CreateThread       
-> CreateListenSocket(WSASocket, bind, listen, setsocketopt(송신 비활성화))-> WSAAccept(blocking)