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


const int32 BUFSIZE = 1000;
struct Session
{
	WSAOVERLAPPED overlapped = {};
	SOCKET socket;
	char recvBuffer[BUFSIZE];
	int32 recvBytes = 0;
	int32 sendBytes = 0;
};

void CALLBACK RecvCallback(DWORD error, DWORD recvLen, LPWSAOVERLAPPED overlapped, DWORD flags)
{
	//TODO
	cout << "Data RecvLen Callback = "<<recvLen << '\n';
	Session* session = (Session*)overlapped;
}

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

	//Overlapped 모델 (Completion Routine 콜백 기반)
	// -비동기 입출력 지원하는 소켓생성
	// -비동기 입출력 함수 호출 (완료 루틴의 시작 주소를 넘겨준다.)
	// -비동기 작업이 바로 완료되지 않으면, WSA_IO_PENDING 오류코드
	// -비동기 입출력 함수 호출한 쓰레드를 ->Alertable Wait 상태로만든다
	// (기다리는 상태 비동기 콜백상태를 호출될때 바뀜.)
	// -비동기 IO 완료되면, 운영체제는 완료 루틴 호출
	// -완료 루틴 호출이 모두 끝나면, 쓰레드 Alertavle wait 상태에서 빠져나옴

	// 1) 오류 발생시 0이 아닌값
	// 2) 전송 바이트 수
	// 3) 비동기 입출력 함수 호출 시 넘겨준 WSAOVERLAPPED 구조체의 주소값.
	// 4) 0 
	// void CompletionRoutine()
	while (true)
	{
		SOCKADDR_IN clientAddr;
		int32 addrLen = sizeof(clientAddr);
		SOCKET clientSocket;

		while (true)
		{
			clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
			if (clientSocket != INVALID_SOCKET)
				break;

			if (::WSAGetLastError() == WSAEWOULDBLOCK)
				continue;

			//뭔가 문제가 있는 항상

			return 0;
		}

		Session session = Session{ clientSocket };
		//WSAEVENT wsaEvent = ::WSACreateEvent();

		cout << "Client Connected!" << '\n';

		while (true)
		{
			WSABUF wsaBuf;
			wsaBuf.buf = session.recvBuffer;
			wsaBuf.len = BUFSIZE;

			DWORD recvLen = 0;
			DWORD flags = 0;

			if(::WSARecv(clientSocket, &wsaBuf, 1, &recvLen, &flags, &session.overlapped, RecvCallback) == SOCKET_ERROR)
			{
				if (::WSAGetLastError() == WSA_IO_PENDING)
				{
					//Pending
					//Alertable Wait 여러함수중 선택
					::SleepEx(INFINITY, TRUE);
					//::WSAWaitForMultipleEvents(1, &wsaEvent, TRUE, WSA_INFINITE, TRUE);

				}
				else {
					//TODO
					break;
				}
			}
			else {
				cout << "Data Recv Len = " << recvLen << '\n';
			}

			cout << "Data Reccv Len = "<<recvLen << '\n';
		}
		::closesocket(session.socket);
		//::WSACloseEvent(wsaEvent);
	}

	::WSACleanup();
}