#include "pch.h"
#include "Service.h"
#include "Session.h"
#include "Lisener.h"

Service::Service(ServiceType type, NetAddress address, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount)
	: _type(type),_netAddress(address),_iocpCore(core),_sessionFactory(factory),_maxSessionCount(maxSessionCount)
{
}

Service::~Service()
{
}

void Service::CloseService()
{
	//TODO
}

SessionRef Service::CreateSession()
{
	SessionRef session = _sessionFactory();
	if (_iocpCore->Register(session) == false)
		return nullptr;

	return session;
}

void Service::AddSession(SessionRef session)
{
	WRITE_LOCK;
	_sessionCount++;
	_sessions.insert(session);
}

void Service::ReleaseSession(SessionRef session)
{
	WRITE_LOCK;
	ASSERT_CRASH(_sessions.erase(session) != 0);
	_sessionCount--;
}

ClientService::ClientService(NetAddress targetAddress, IocpCoreRef core, SessionFactory factory, int32 MaxSessionCount)
	:Service(ServiceType::Client,targetAddress,core,factory,MaxSessionCount)
{

}

bool ClientService::Start()
{
	//TODO
	return true;
}

ServerService::ServerService(NetAddress address, IocpCoreRef core, SessionFactory factory, int32 MaxSessionCount)
	:Service(ServiceType::Server, address, core, factory, MaxSessionCount)
{
}

bool ServerService::Start()
{
	if (CanStart() == false)
		return false;

	_listener = MakeShared<Listener>();
	if (_listener == nullptr)
		return false;

	ServerServiceRef service = static_pointer_cast<ServerService>(shared_from_this());
	if (_listener->StartAccept(service) == false)
		return false;

	return true;
}

void ServerService::CloseService()
{
	//TODO
	Service::CloseService();
}
