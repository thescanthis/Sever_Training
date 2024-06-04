#include "pch.h"
#include <iostream>

//
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

bool Socket_Error(SOCKET clientSocket)
{
	if (clientSocket == INVALID_SOCKET)
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
	//Winsock 초기화 서버와 클라 동일함 초기화부분
	//관련 정보가 wsaData에 채워짐
	WSAData wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData))
		return 0;

#if 주석
	클라와 서버 둘다 같음.
	//1. 소켓을 만들어야함.
	//소문자는 리눅스에서도 사용할수있을 가능성이 높음.
	//ad :AF_INET = IPv4,AF_INET6=IPv6
	//TCP = SOCK_STREAM,UDP = SOCK_DGRAM
	//protocol = 0
	//return : descriptor
#endif
	SOCKET clientSocket = ::socket(AF_INET,SOCK_STREAM,0);
	Socket_Error(clientSocket);

	SOCKADDR_IN serverAddr; // IPv4
	SockAddr_In_Init(serverAddr);

	if (::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr))==SOCKET_ERROR)
	{
		Socket_Error(clientSocket);
	}

	cout << "Connected To Server!" << '\n';

	while (true)
	{
		//TODO
		this_thread::sleep_for(1s);
	}

	::closesocket(clientSocket);
	::WSACleanup();
	return 0;
}