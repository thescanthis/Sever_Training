#include "pch.h"
#include "IocpCore.h"
#include "IocpEvent.h"

IocpCore GlocpCore;

IocpCore::IocpCore()
{
	_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	ASSERT_CRASH(_iocpHandle != INVALID_HANDLE_VALUE);
}

IocpCore::~IocpCore()
{
	::CloseHandle(_iocpHandle);
}

bool IocpCore::Register(IocpObject* iocpObject)
{
	return CreateIoCompletionPort(iocpObject->GetHandle(), _iocpHandle,/*Key*/0, 0);
}

bool IocpCore::Dispatch(uint32 timeout)
{
	//일감이 있는지 두리번 거림
	DWORD numOfBytes = 0;
	ULONG_PTR Key = 0;
	IocpObject* iocpObject = nullptr;
	IocpEvent* iocpEvnet = nullptr;

	if (::GetQueuedCompletionStatus(_iocpHandle, OUT & numOfBytes, OUT &Key, OUT reinterpret_cast<LPOVERLAPPED*>(&iocpEvnet), timeout))
	{
		iocpObject->Dispatch(iocpEvnet, numOfBytes);
	}
	else
	{
		int32 errCode = ::WSAGetLastError();

		switch (errCode)
		{
		case WAIT_TIMEOUT:
			return false;
		default:
			//TODO 로그찍기
			iocpObject->Dispatch(iocpEvnet, numOfBytes);
			break;
		}

	}
	return false;
}
