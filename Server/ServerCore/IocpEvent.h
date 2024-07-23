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
	EventType GetType() { return _type; }

private:
	EventType _type;
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
	void SetSession(Session* session) { _session = session; }
	Session* GetSession() { return _session; }
public:
	Session* _session = nullptr;
};

/*---------------------
	  RecvEvnet
---------------------*/
class RecvEvnet : public IocpEvent
{
public:
	RecvEvnet() : IocpEvent(EventType::Recv) {}
};

/*---------------------
	  SendEvnet
---------------------*/
class SendEvnet : public IocpEvent
{
public:
	SendEvnet() : IocpEvent(EventType::Send) {}
};