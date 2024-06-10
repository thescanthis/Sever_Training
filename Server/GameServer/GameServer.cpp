#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "ThreadManager.h"


#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

bool Socket_Error(SOCKET listenSocket,const char* cause)
{
	if (listenSocket == INVALID_SOCKET)
	{
		int32 errCode = ::WSAGetLastError();
		cout << cause << "ErrorCode:" << errCode << '\n';
		return false;
	}
	return true;
}

void SockAddr_In_Init(SOCKADDR_IN& serverAddr)
{
	//연결할 목적지 ? (IP주소 + Port)
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY); // 니가 알아서 해줘
	serverAddr.sin_port = ::htons(7777);
}

int main()
{
	WSAData wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return 0;

	SOCKET serverSocket = ::socket(AF_INET, SOCK_DGRAM, 0);
	if(!Socket_Error(serverSocket, "Socket"))
		return 0;

	//옵션을 해석하고 처리할 주체
	//소켓코드 ->SOL_SOCKET
	//IPv4 ->IPPROTO_IP
	//TCP 프로토콜 -> IPPROTO_TCP
	
	//SO_KEEPALIVE = 주기적으로 연결 상태 확인 (TCP Only)
	//상대방이 소리소문없이 연결을 끊는경우가 있음.
	bool enable = true;
	::setsockopt(serverSocket,SOL_SOCKET,SO_KEEPALIVE,(char*)&enable,sizeof(enable));

	//SO_LINGER = 지연하다
	//소인 버퍼에 있는 데이터를 보낼것인가, 날릴것인가?
	//onoff = 0이면 closesocket() 바로 리턴, 아니면 linger초만큼 대기
	LINGER linger;
	linger.l_onoff = 1;
	linger.l_linger = 5;
	::setsockopt(serverSocket, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));
	
	//Half-close
	//::shutdown(serverSocket, SD_SEND);
	//::closesocket(serverSocket);
	
	//SO_SNDBUF
	//SO_RCVBUF

	int32 sendBuffsize;
	int32 optionLen = sizeof(sendBuffsize);
	::getsockopt(serverSocket, SOL_SOCKET, SO_SNDBUF, (char*)&sendBuffsize, &optionLen);
	
	cout << sendBuffsize<<'\n';

	int32 recvBuffersize;
	optionLen = sizeof(recvBuffersize);
	::getsockopt(serverSocket, SOL_SOCKET, SO_RCVBUF, (char*)&recvBuffersize, &optionLen);
	cout << recvBuffersize << '\n';

	::WSACleanup();
}