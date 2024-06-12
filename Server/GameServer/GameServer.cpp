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

	// WSAEventSelect = (WSAEventSelect 함수가 핵심이 되는)
	// 소켓과 관련된 네트워크 이벤트를 이벤트객체를 통해 감지.
	// 비동기방식이고 이벤트를 통해 감지를 함.

	// 이벤트 객체 관련 함수
	// 생성 : WSACreateEvent(수동 리셋 Manual-Reset + Non-Signaled 상태시작)
	// 삭제 : WSACloseEvent
	// 신호상태 감지 : WSAWaitForMultipleEvents
	// 구체적인 네트워크 이벤트 : WSAEnumnetworkEvents

	//소켓 갯수만큼 이벤트를 연동
	//소켓 <-> 이벤트 객체연동 
	//WSAEventSelect(socket,event,networkEvents);
	//FD_ACCEPT : 접속한 클라가 있다면 accept
	//FD_READ   : 데이터 수신 가능 recv,recvfrom
	//FD_WRITE  : 데이터 송신 가능 send,sendto
	//FD_CLOSE  : 상대 접속종료
	//FD_CONNECT: 통신을 위한 연결절차완료
	//FD_OOB

	//주의사항
	//WSAEventSelect 함수를 호출하면,해당소켓은 자동으로 넌블로킹 모드 전환
	//accept() 함수가 리턴하는 소켓은 listenSocket과 동일한 속성을 갖는다
	// -따라서 clientSocket은 FD_READ,FD_WRITE등을 다시 등록 필요
	// 드물게 WSAEWOULDBLOCK 오류가 뜰수있으니, 예외처리 필요
	// - 이벤트 발생시, 적절한 소켓 함수 호출해야함.
	// - 아니면, 다음번에는 동일 네트워크 이벤트가 발생X
	// ex)FD_READ 이벤트 떳으면 recv() 호출 ,안하면 FD_READ 다시 호출되지않음.

	//1. count,event
	//2. waitAll : 모두 기다림,하나만 완료되어도 OK
	// timeout
	// 지금은 false
	// return : 완료되면 첫번째 인덱스
	// WSAWaitForMultipleEvents

	//1. socket
	//2. eventObejct : socket 과 연동된 이벤트 객체 핸들을 넘겨주면, 이벤트 객체를 nonsignaled
	//WSAEnumNetworkEvents

	vector<WSAEVENT> wsaEvents;
	vector<Session> sessions;
	sessions.reserve(100);

	WSAEVENT litenEvent = ::WSACreateEvent();
	wsaEvents.push_back(litenEvent);
	sessions.push_back(Session({ listenSocket }));


	if (WSAEventSelect(listenSocket, litenEvent, FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR)
		return 0;

	while (true)
	{
		//[] [] [] 
		int32 index = ::WSAWaitForMultipleEvents(wsaEvents.size(),&wsaEvents[0],FALSE,WSA_INFINITE,FALSE);
		
		//뭔가 문제가 있다면?
		if (index == WSA_WAIT_FAILED)
			continue;

		index -= WSA_WAIT_EVENT_0;
		
		//EnumNetworkEvents에 이미 있긴함 해도 그만 안해도그만
		//::WSAResetEvent(wsaEvents[index]);

		WSANETWORKEVENTS networkEvents;
		//세션의 소켓을 꺼내서 wsaevents의 핸들을 사용해서, 네트워크 구조체에 해당 결과물을 추가
		if (::WSAEnumNetworkEvents(sessions[index].socket, wsaEvents[index], &networkEvents))
		continue;

		// Listener 소켓 체크
		// 비트값이 같다면
		if (networkEvents.lNetworkEvents & FD_ACCEPT)
		{
			//Error-Check
			if (networkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
				continue;

			SOCKADDR_IN clientAddr;
			int32 addLen = sizeof(clientAddr);
			SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addLen);
			if (clientSocket != INVALID_SOCKET)
			{
				cout << "Client Connect" << '\n';
				WSAEVENT clientEvent = ::WSACreateEvent();
				wsaEvents.push_back(clientEvent);
				sessions.push_back(Session{ clientSocket });
				// | 둘중 하나라도 같으면 전부 비트를 1로 반환.
				if (::WSAEventSelect(clientSocket, clientEvent, FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
					return 0;
			}
		}

		//ClientSession 소켓 체크
		if (networkEvents.lNetworkEvents & FD_READ || networkEvents.lNetworkEvents & FD_WRITE)
		{
			//Error-Check
			if ((networkEvents.lNetworkEvents & FD_READ) && (networkEvents.iErrorCode[FD_READ_BIT]!=0))
				continue;
			if ((networkEvents.lNetworkEvents & FD_WRITE) && (networkEvents.iErrorCode[FD_READ_BIT]!=0))
				continue;

			Session& s = sessions[index];

			//Read
			if (s.recvBytes == 0)
			{
				int32 recvLen = ::recv(s.socket, s.recvBuffer, BUFSIZE, 0);
				if (recvLen == SOCKET_ERROR && ::WSAGetLastError() != WSAEWOULDBLOCK)
				{
					//TODO
					continue;
				}

				s.recvBytes = recvLen;
				cout << "Recv Data = " << recvLen << '\n';

				//Write
				//아직 보낼 데이터가 남아 있따면
				if (s.recvBytes > s.sendBytes)
				{
					int32 sendLen = ::send(s.socket, &s.recvBuffer[s.sendBytes], s.recvBytes - s.sendBytes, 0);

					if (sendLen == SOCKET_ERROR && WSAGetLastError() && WSAEWOULDBLOCK)
					{
						//TODO
						continue;
					}

					s.sendBytes += sendLen;
					if (s.recvBytes == s.sendBytes)
					{
						s.recvBytes = 0;
						s.sendBytes = 0;
					}

					cout << "Send Data = " << recvLen << '\n';
				}
			}
		}

		if (networkEvents.lNetworkEvents & FD_CLOSE)
		{
			//TODO : Remove Socket
		}
	}

	::WSACleanup();
}