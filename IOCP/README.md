## IOCP 

# Overlapped 방식 3개
Event, Callback, IOCP**  

# 헤더 정보
IOCP https://learn.microsoft.com/en-us/windows/win32/api/ioapiset/  
<WinSock2.h> https://learn.microsoft.com/en-us/windows/win32/api/winsock2/  
<WS2tcpip.h> https://learn.microsoft.com/en-us/windows/win32/api/ws2tcpip/  

#pragma comment(lib, "Ws2_32.lib")

# 예제
https://github.com/microsoft/Windows-classic-samples/blob/main/Samples/Win7Samples/netds/winsock/iocp/server/IocpServer.Cpp  

# 주요 구조 흐름

메인 스레드  
WSAStartup      WinSockAPI Startup  
-> CreateIoCompletionPort -> CreateThread   워킹스레드(수신처리) 생성
-> CreateListenSocket(WSASocket, bind, listen, setsocketopt(송신 비활성화))-> WSAAccept(blocking) 블로킹 방식으로 클라이언트 접속 처리  
-> UpdateCompletionPort -> WSARecv 소켓 IOCP 등록 및 비동기 수신 시작, WSAAccept 루프  
WSAAccept는 비동기적으로 스레드가 blocking 하기 때문에, Accept, Connect도 비동기로 하려면 mwssock.h의 AcceptEx 함수를 써야함

워킹 스레드  

GetQueuedCompletionStatus로 큐에서 소켓 받아옴
switch( lpIOContext->IOOperation ) 저장된 컨텍스트의 IOOperation값에 따른 처리
ClientIoRead일 경우 최근 해당 소켓에 WSARecv했다는 것(해당 연산 전에 값을 설정해둠), 받은 데이터 그대로 WSASend로 재전송
ClientIoWrite일 경우 최근 연산이 WSASEnd라는 것, 송신 바이트 수에 따라 다시 Send하든 Recv하든 분기 갈림  

# 실습, BroadCasting Server
서버가 클라이언트로부터 char...(ASCII) 받고서 브로드캐스팅. 