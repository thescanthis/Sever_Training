#include "pch.h"
#include "ThreadManager.h"
#include "Service.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "BufferWriter.h"
#include "ClientPacketHandler.h"
#include "Protocol.pb.h"
#include "job.h"
#include "Room.h"
#include "Player.h"

#include <functional>


void HealByValue(int64 target, int32 value)
{
	std::cout << target << "한테 힐" << value << "만큼 줌" << '\n';
}

class Knight
{
public:
	void HealMe(int32 value)
	{
		cout << "HealMe!" << value << '\n';
	}
};

int main()
{
	ClientPacketHandler::Init();

	ServerServiceRef service = MakeShared<ServerService>(
		NetAddress(L"127.0.0.1", 7777),
		MakeShared<IocpCore>(),
		MakeShared<GameSession>, // TODO : SessionManager 등
		100);

	ASSERT_CRASH(service->Start());

	for (int32 i = 0; i < 5; i++)
	{
		GThreadManager->Launch([=]()
			{
				while (true)
				{
					service->GetIocpCore()->Dispatch();
				}
			});
	}

	GThreadManager->Join();
}