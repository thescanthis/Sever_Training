#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"
#include "Service.h"
Session::Session()
{
	_socket = SocketUtils::CreateSocket();
}

Session::~Session()
{
	SocketUtils::Close(_socket);
}

void Session::Send(BYTE* buffer, int32 len)
{
	//생각할 문제
	// 1)버퍼관리?
	// 2)sendEvent 관리? 단일? 여러개? WSASend 중첩?

	//TEMP
	SendEvnet* sendEvent = xnew<SendEvnet>();
	sendEvent->owner = shared_from_this(); //ADD_REF
	sendEvent->buffer.resize(len);
	memcpy(sendEvent->buffer.data(), buffer, len);

	WRITE_LOCK;
	RegisterSend(sendEvent);
}

void Session::DisConnect(const WCHAR* cause)
{
	if (_connected.exchange(false) == false)
	{
		return;
	}

	wcout << "Disconnect : " << cause << '\n';
	OnDisconnected();
	SocketUtils::Close(_socket);
	GetService()->ReleaseSession(GetSessionRef());
}

HANDLE Session::GetHandle()
{
	return reinterpret_cast<HANDLE>(_socket);
}

void Session::Dispatch(IocpEvent* iocpEvent, int32 numOfBytes)
{
	switch (iocpEvent->eventType)
	{
	case EventType::Connect:
		ProcessConnect();
		break;
	case EventType::Recv:
		ProcessRecv(numOfBytes);
		break;
	case EventType::Send:
		ProcessSend(static_cast<SendEvnet*>(iocpEvent), numOfBytes);
		break;
	default:
		break;
	}
}

void Session::RegisterConnect()
{
}

void Session::RegisterRecv()
{
	if (IsConnected() == false)
		return;

	//이렇게 해도되지만 매번 생성하고 지우고 하는거보단
	//RecvEvent* recvEvent = xnew<RecvEvent>();
	//섹션 마다 맴버변수로 가지고있는것이 좀더 편함.
	_recvEvent.Init();
	_recvEvent.owner = shared_from_this(); //ADD_REF

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer);
	wsaBuf.len = len32(_recvBuffer);

	DWORD numOfBytes = 0;
	DWORD flags = 0;
	if (SOCKET_ERROR == ::WSARecv(_socket, &wsaBuf, 1, OUT & numOfBytes, OUT & flags, &_recvEvent, nullptr))
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			HandleError(errorCode);
			_recvEvent.owner = nullptr; //RELEASE_REF
		}
	}
}

void Session::RegisterSend(SendEvnet* sendEvent)
{
	if (IsConnected() == false)
		return;

	WSABUF wsaBuf;
	wsaBuf.buf = (char*)sendEvent->buffer.data();
	wsaBuf.len = (ULONG)sendEvent->buffer.size();

	DWORD numOfBytes = 0;

	if (SOCKET_ERROR == ::WSASend(_socket, &wsaBuf, 1, OUT & numOfBytes, 0, sendEvent, nullptr))
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			HandleError(errorCode);
			sendEvent->owner = nullptr; //ERLEASE_REF
			xdelete(sendEvent);
		}
	}
}

void Session::ProcessConnect()
{
	_connected.store(true);

	// 세션 등록
	GetService()->AddSession(GetSessionRef());

	//컨텐츠 코드에서 오버로딩
	OnConnected();

	//수신 등록
	RegisterRecv();
}

void Session::ProcessRecv(int32 numOfBytes)
{
	_recvEvent.owner = nullptr; //RELEASE_REF

	if (numOfBytes == 0)
	{
		DisConnect(L"Recv 0");
		return;
	}

	//컨텐츠 코드에서 오버로딩
	OnRecv(_recvBuffer, numOfBytes);

	//여기까지 왔으면 통지가 완료된거니까 다시 수신등록을 해줌
	RegisterRecv();
}

void Session::ProcessSend(SendEvnet* sendEvent,int32 numOfBytes)
{
	sendEvent->owner = nullptr;
	xdelete(sendEvent);

	if (numOfBytes == 0)
	{
		DisConnect(L"Send 0");
		return;
	}

	//컨텐츠 코드에서 오버로딩
	OnSend(numOfBytes);
}

void Session::HandleError(int32 errorCode)
{
	switch (errorCode)
	{
	case WSAECONNRESET:
		break;
	case WSAECONNABORTED:
		DisConnect(L"HandleError");
		break;
	default:
		//TODO
		cout << "Handle Error :" << errorCode << '\n';
		break;
	}
}
