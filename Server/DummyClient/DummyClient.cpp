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
	SOCKET clientSocket = ::socket(AF_INET,SOCK_DGRAM,0);
	Socket_Error(clientSocket,"Socket");

	SOCKADDR_IN serverAddr; // IPv4
	SockAddr_In_Init(serverAddr);

	while (true)
	{
		//클라 입장에서는 커널단계로 보내주기만 하면 성공적으로 보냈다고 생각
		// 그래서 서버가 recv해주지않아도 넘어가는 이유를 알 수 있다.
		//TODO
		char snedBuffer[100] = "Hello World";
		int32 res = ::sendto(clientSocket, snedBuffer, sizeof(snedBuffer), 0,
			(SOCKADDR*)&serverAddr,sizeof(serverAddr));
		if (!Socket_Error(res,"SendTo"))
			return 0;

		cout << "Send Data" <<sizeof(snedBuffer)<<'\n';

		SOCKADDR_IN recvAddr;
		::memset(&recvAddr, 0, sizeof(recvAddr));
		int32 addLen = sizeof(recvAddr);

		char recvBuffer[1000];

		int32 recvLen = ::recvfrom(clientSocket, recvBuffer, sizeof(recvBuffer), 0,
		(SOCKADDR*)&recvAddr,&addLen);


		if (recvLen <= 0)
		{
			if (Socket_Error(recvLen,"RecvTo"))
				return 0;
		}
		cout << "Recv Data! Data = " << recvLen << '\n';
		cout << "Recv Data! Len = " << recvLen << '\n';

		this_thread::sleep_for(1s);
	}

	::closesocket(clientSocket);
	::WSACleanup();
	return 0;
}