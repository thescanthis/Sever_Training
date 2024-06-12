#include "pch.h"
#include <iostream>

//
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

bool Socket_Error(SOCKET clientSocket, const char* cause)
{
	if (clientSocket == INVALID_SOCKET)
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
	//serverAddr.sin_addr.s_addr = ::inet_addr("127.0.0.1");
	::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
	serverAddr.sin_port = ::htons(7777);
	//host to network short = 내컴퓨터에서 네트워크에 알맞게 고침
	//Little-Enddian vs Big-Endian
	//[0x78][0x56][0x34][0x12] = little
	//[0x12][0x34][0x56][0x78] = big = network 선호
}

int main()
{
	this_thread::sleep_for(1s);
	WSAData wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return 0;

	SOCKET clientSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET)
		return 0;

	u_long on = 1;
	if (::ioctlsocket(clientSocket, FIONBIO, &on))
		return 0;

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	::inet_pton(AF_INET, "127.0.0.1",&serverAddr.sin_addr);
	serverAddr.sin_port = ::htons(7777);

	//Connect
	while (true)
	{
		if (::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		{
			//원랜 블록이여야 했는데....

			if (::WSAGetLastError() == WSAEWOULDBLOCK)
				continue;

			if (::WSAGetLastError() == WSAEISCONN)
				break;
			//Error;
			break;
		}
	}
	cout << "Connected to Server !"<<'\n';

	char sendBuffer[100] = "Hello World";

	//Send
	while (true)
	{
		if (::send(clientSocket, sendBuffer, sizeof(sendBuffer), 0) == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
				continue;
			
			//Error
			break;
		}
		cout << "Send Data ! Len = " << sizeof(sendBuffer) << '\n';

		while (true)
		{
			char recvBuffer[1000];
			int32 recvLen = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
			if (recvLen == SOCKET_ERROR)
			{
				if (WSAGetLastError() == WSAEWOULDBLOCK)
					continue;

				//Error
				break;
			}
			else if (recvLen == 0)
			{
				//데이터가 아예 안옴.
				break;
			}

			cout << "성공!" << '\n';
			break;
		}

		this_thread::sleep_for(1s);
	}

	::closesocket(clientSocket);
	::WSACleanup();
}