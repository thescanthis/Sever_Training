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
	//Winsock 초기화
	//관련 정보가 wsaData에 채워짐
	WSAData wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData))
		return 0;

	//UDP
	SOCKET serverSocket = ::socket(AF_INET, SOCK_DGRAM, 0);

	if (!Socket_Error(serverSocket, "Socket"))
		return 0;
	SOCKADDR_IN serverAddr;
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
	serverAddr.sin_port = ::htons(7777);

	if (::bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		if (!Socket_Error(serverSocket, "Bind"))
			return 0;
	}

	while (true)
	{
		SOCKADDR_IN clinetAddr;
		::memset(&clinetAddr, 0, sizeof(clinetAddr));
		int32 addLen = sizeof(clinetAddr);

		char buffer[1000] = {};

		int32 recvLen = ::recvfrom(serverSocket, buffer, sizeof(buffer), 0,
			(SOCKADDR*)&clinetAddr, &addLen);

		if (recvLen <= 0)
		{
			if (!Socket_Error(serverSocket, "RecvFrom"))
				return 0;
		}

		cout << "Recv Data! Data=" << buffer << '\n';
		cout << "Recv Data! Len=" << recvLen << '\n';

		int32 errorCode = ::sendto(serverSocket, buffer, recvLen, 0,
			(SOCKADDR*)&clinetAddr, sizeof(clinetAddr));

		if (errorCode == SOCKET_ERROR)
		{
			if (!Socket_Error(serverSocket, "SendTo"))
				return 0;
		}
	}

	::WSACleanup();
	return 0;
}