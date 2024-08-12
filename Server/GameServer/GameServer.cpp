#include "pch.h"
#include "ThreadManager.h"
#include "Service.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "BufferWriter.h"
#include "ServerPacketHandler.h"

// 패키직렬화

class Player
{
public:
	int32 hp = 0;
	int32 attack = 0;
	Player* target = nullptr;
	vector<int>buffs;
};

int main()
{
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

	char SendData[] = "Hello World";
	while (true)
	{
		vector<BuffData> buffs{ BuffData{100,1.5f},BuffData{200,2.3f},BuffData{300,0.7f} };
		SendBufferRef sendBuffer = ServerPacketHandler::Make_S_TEST(1001, 100, 10, buffs,L"안녕하세요");

		GSessionManager.Broadcast(sendBuffer);
		this_thread::sleep_for(250ms);
	}

	GThreadManager->Join();
}