#pragma once

/*-------------
	IocpCore
-------------*/

class IocpObject
{
public:
	virtual HANDLE GetHandle() abstract;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) abstract;
};

class IocpCore
{
public:
	IocpCore();
	~IocpCore();

	HANDLE GetHandle() {
		return _iocpHandle;
	};

	bool Register(class IocpObject* iocpObject);
	bool Dispatch(uint32 timeout = INFINITE);

private:
	HANDLE _iocpHandle;

};


extern IocpCore GlocpCore;
