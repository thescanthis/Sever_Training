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
	WSAData wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return 0;

	SOCKET clientSocket = ::socket(AF_INET, SOCK_DGRAM, 0);
	if (!Socket_Error(clientSocket, "Socket"))
		return 0;


	::closesocket(clientSocket);
	::WSACleanup();
}