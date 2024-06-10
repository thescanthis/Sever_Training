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

	//블로킹(Blocking) 소켓
	//accept -> 접속한 클라가 있을때
	//connect -> 서버접속 성공 했을때
	//send,sendto -> 요청한데이터를 송신 버퍼에 복사 했을때
	//recv,recvfrom -> 수신버퍼에 도착한 데이터가 있꼬,이를 유저레벨 버퍼에 복사 했을때

	//논블로킹 (Non-Blocking)

	SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET)
		return 0;

	u_long on = 1;
	if (::ioctlsocket(listenSocket, FIONBIO, &on))
		return 0;

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
	serverAddr.sin_port = ::htons(7777);

	if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		return 0;

	if (::listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
		return 0;

	cout << "Accept" << '\n';

	SOCKADDR_IN clientAddr;
	int32 addrLen = sizeof(clientAddr);

	//Accept
	while (true)
	{
		SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			//Non-Blocking에선 여기로 들어가도 문제가 있는지는 모름 다시 체크를 해야함.
			if (::WSAGetLastError() == WSAEWOULDBLOCK)
				continue;

			//Error
			break;
		}

		cout << "Client Connected!" << '\n';
		//Recv

		while (true)
		{
			char recvBuffer[1000];
			int recvLen = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
			if (recvLen == SOCKET_ERROR)
			{
				if (::WSAGetLastError() == WSAEWOULDBLOCK)
					continue;

				break;
			}
			else if (recvLen == 0)
				break;
			cout << "Recv Data Len = " << recvLen << '\n';

			//Send
			while (true)
			{
				if (::send(clientSocket, recvBuffer, sizeof(recvLen), 0) == SOCKET_ERROR)
				{
					if (::WSAGetLastError() == WSAEWOULDBLOCK)
						continue;
					break;
				}
				cout << "Send Data ! Len = "<<recvLen << '\n';
				break;
			}

		}
	}

	::WSACleanup();
}