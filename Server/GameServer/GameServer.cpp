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

enum IO_TYPE
{
	READ,
	WRITE,
	ACCEPT,
	CONNECT
};

struct OverlappedEx
{
	WSAOVERLAPPED overlapped = {};
	int32 type = 0;
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

void WorkerThreadMain(HANDLE iocpHandle)
{
	while (true)
	{
		DWORD bytes = 0;
		Session* session = nullptr;
		OverlappedEx* overlappedEx = nullptr;

		//일감을 받을때 까진 대기
		BOOL ret = ::GetQueuedCompletionStatus(iocpHandle, &bytes, 
			(ULONG_PTR*)&session, (LPOVERLAPPED*)overlappedEx, INFINITY);

		if (ret == FALSE || bytes) continue;

		ASSERT_CRASH(overlappedEx->type == IO_TYPE::READ);

		cout << "Recv Data IOPC = "<<bytes << '\n';
		WSABUF wsabuf;
		wsabuf.buf = session->recvBuffer;
		wsabuf.len = BUFSIZE;

		DWORD recvLen = 0;
		DWORD flags = 0;
		::WSARecv(session->socket,&wsabuf, 1, &recvLen, &flags, &overlappedEx->overlapped, NULL);
	}
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
	// - 비동기 입출력 함수 완료시, 쓰레드마다 APC큐에 일감이 쌓임.
	// - Alertable Wait상태로 들어가서 APC큐를 비우기(콜백함수)
	// 단점) APC큐가 쓰레드마다 고유하게 있다! Alertable Wait 자체 부담!
	// 단점) 이벤트방식 : (소켓 : 이벤트 1:1 대응)

	//IOCP(Completion Port) 모델
	// - Completion Port(쓰레드마다 있는건 아니고, 1개가 중앙에서 APC큐를 관리 하는 그림)
	// - Alertable Wait->CP결과처리를 GetQueueCOmpletionStatus
	// 쓰레드랑 궁합이 굉장히 좋다.

	//CreateCompletionPort
	//GetQueuedCompletionStatus

	vector<Session*> sessionManager;

	//CP 생성 초기화
	HANDLE iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	for (int32 i = 0; i < 5; i++)
	{
		GThreadManager->Launch([=]() {WorkerThreadMain(iocpHandle); });
	}
	//Main Thread = Accept
	while (true)
	{
		SOCKADDR_IN clientAddr;
		int32 addrLen = sizeof(clientAddr);
		SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);

		if (clientSocket == INVALID_SOCKET)
			return 0;

		Session* session = new Session();
		session->socket = clientSocket;
		sessionManager.push_back(session);
		//WSAEVENT wsaEvent = ::WSACreateEvent();

		cout << "Client Connected!" << '\n';

		//소켓을 CP에 등록
		::CreateIoCompletionPort((HANDLE)clientSocket,iocpHandle, (ULONG_PTR)session,0);

		WSABUF wsaBuf;
		wsaBuf.buf = session->recvBuffer;
		wsaBuf.len = BUFSIZE;

		DWORD recvLen = 0;
		DWORD flags = 0;
		OverlappedEx* overlappedEx = new OverlappedEx();
		overlappedEx->type = IO_TYPE::READ;
		::WSARecv(clientSocket, &wsaBuf, 1, &recvLen, &flags, &overlappedEx->overlapped, NULL);
		//::closesocket(session.socket);
		//::WSACloseEvent(wsaEvent);
	}

	GThreadManager->Join();
	::WSACleanup();
}