#pragma once

class Session;

enum class EventType : uint8
{
	Connect,
	Accept,
	/*PreRecv 고오오급 기법*/
	Recv,
	Send
};

//virtual로 사용 하게 되면 offset 0번의 주소값이 바뀔수 있어서 조심해야함.

/*----------------
	IocpEvent
----------------*/

class IocpEvent : public OVERLAPPED
{
public:
	IocpEvent(EventType type);

	void Init();
public:

	EventType		eventType;
	IocpObjectRef	owner;
};

/*---------------------
	ConnectEvnet
---------------------*/
class ConnectEvent : public IocpEvent
{
public:
	ConnectEvent() : IocpEvent(EventType::Connect) {}
};

/*---------------------
	  AcceptEvnet
---------------------*/

class AcceptEvent : public IocpEvent
{
public:
	AcceptEvent() : IocpEvent(EventType::Accept) {}
public:
	SessionRef session = nullptr;

};

/*---------------------
	  RecvEvnet
---------------------*/
class RecvEvent : public IocpEvent
{
public:
	RecvEvent() : IocpEvent(EventType::Recv) {}
};

/*---------------------
	  SendEvnet
---------------------*/
class SendEvnet : public IocpEvent
{
public:
	SendEvnet() : IocpEvent(EventType::Send) {}

	//TEMP
	vector<BYTE> buffer;
};