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

bool Socket_Error(SOCKET listenSocket)
{
	if (listenSocket == INVALID_SOCKET)
	{
		int32 errCode = ::WSAGetLastError();
		cout << "Socket ErrorCode" << errCode << '\n';
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
	//Winsock 초기화
	//관련 정보가 wsaData에 채워짐
	WSAData wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData))
		return 0;


	SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	Socket_Error(listenSocket);

	SOCKADDR_IN serverAddr; // IPv4
	SockAddr_In_Init(serverAddr);

	//안내원 폰 개통! 식당의 대표번호
	if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		Socket_Error(listenSocket);
	}

	//영업 시작!
	if(::listen(listenSocket, 10) == SOCKET_ERROR)
	{
		Socket_Error(listenSocket);
	}

	while (true)
	{
		SOCKADDR_IN clientAddr;
		::memset(&clientAddr, 0, sizeof(clientAddr));

		int32 addLen = sizeof(clientAddr);
		SOCKET clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &addLen);
		Socket_Error(clientSocket);

		//손님 입장
		char ipAddress[16];
		::inet_ntop(AF_INET, &clientAddr.sin_addr, ipAddress, sizeof(ipAddress));
		cout << "Client Connected! IP = " << ipAddress << '\n';

		while (true)
		{
			char recvBuffer[1000];
			int32 recvLen = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);

			if (recvLen <= 0)
			{
				if (Socket_Error(recvLen))
					return 0;
			}

			cout << "Recv Data! Data = " << recvLen << '\n';
			cout << "Recv Data! Len = " << recvLen << '\n';

			//상대방이 데이터를 그대로 다시 토스
			int32 recvLenCode = ::send(clientSocket, recvBuffer, recvLen, 0);

			if (recvLenCode <= 0)
			{
				if (Socket_Error(recvLenCode))
					return 0;
			}
		}
	}

	::WSACleanup();
	return 0;
}