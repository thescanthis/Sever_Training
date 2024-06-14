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
	SOCKET socket;
	char recvBuffer[BUFSIZE];
	int32 recvBytes = 0;
	int32 sendBytes = 0;
	WSAOVERLAPPED overlapped = {};
};

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

	//Overlapped IO (비동기 + 논블로킹)
	// - Overlapped 함수를 건다(WSARecv,WSASend)
	// - Overlapped 함수가 성공 했는지 확인
	// -> 성공 했으면 결과얻어서 처리
	// -> 실패했으면 사유를 확인. Pending상태인지 에러상태인지 확인을 해야함.

	// 1) 비동기 입출력 소켓
	// 2) WSABUF 시작주소 + 개수
	// 3) 보내고/받은 바이트수
	// 4) 상세옵션 0
	// 5) WSAOVERAPPED 구조체 주소값.				 방식 1
	// 6) 입출력이 완료되면 OS가 호출할 콜백 함수   방식 2
	// WSASend
	// WSARecv
	//Scatter-Gather

	//Overlapped 모델 (이벤트 기반)
	// -비동기 입출력 지원하는 소켓 생성 + 통지받기위한 이벤트 객체 생성
	// -비동기 입출력 함수 호출(1에서 만든 이벤트 객체를 같이 넘겨줌)
	// -비동기 작업이 바로 완료되지않으면 WSA_IO_PENDING 오류코드
	// 운영체제는 이벤트 객체를 signaled 상태로 만들어서 완료 상태 알려줌
	// -WSAWaitForMultipleEvents 함수 호출해서 이벤트 객체의 signal 판별
	// -WSAGetoverlappedResult 호출해서 비동기 입출력 결과 확인 및 데이터 처리.

	// 비동기소켓,overlapped구조체,바이트수,대기확인,거의사용 X

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
		WSAEVENT wsaEvent = ::WSACreateEvent();
		session.overlapped.hEvent = wsaEvent;

		cout << "Client Connected!" << '\n';

		while (true)
		{
			WSABUF wsaBuf;
			wsaBuf.buf = session.recvBuffer;
			wsaBuf.len = BUFSIZE;

			DWORD recvLen = 0;
			DWORD flags = 0;

			if(::WSARecv(clientSocket, &wsaBuf, 1, &recvLen, &flags, &session.overlapped, nullptr) == SOCKET_ERROR)
			{
				if (::WSAGetLastError() == WSA_IO_PENDING)
				{
					::WSAWaitForMultipleEvents(1, &wsaEvent, TRUE, WSA_INFINITE, FALSE);
					::WSAGetOverlappedResult(session.socket, &session.overlapped, &recvLen, FALSE, &flags);
				}
				else {
					//TODO
					break;
				}
			}

			cout << "Data Reccv Len = "<<recvLen << '\n';
		}
		::closesocket(session.socket);
		::WSACloseEvent(wsaEvent);
	}

	::WSACleanup();
}