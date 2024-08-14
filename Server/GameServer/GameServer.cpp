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

	WCHAR sendData3[1000] = L"가"; // UTF16 = Unicode (한글/로마 2바이트)
	while (true)
	{
		PKT_S_TEST_WRITE pktWriter(1001, 100, 10);
		PKT_S_TEST_WRITE::BuffsList buffList = pktWriter.ReserveBuffsList(3);
		buffList[0] = { 100,1.5f };
		buffList[1] = { 200,2.3f };
		buffList[2] = { 300,0.75f };
		
		PKT_S_TEST_WRITE::BuffsVictimesList vic0 =  pktWriter.ReverseBuffsVictimList(&buffList[0], 3);
		{
			vic0[0] = 1000;
			vic0[1] = 2000;
			vic0[2] = 3000;
		}

		PKT_S_TEST_WRITE::BuffsVictimesList vic1 = pktWriter.ReverseBuffsVictimList(&buffList[1], 1);
		{
			vic1[0] = 1000;
		}

		PKT_S_TEST_WRITE::BuffsVictimesList vic2 = pktWriter.ReverseBuffsVictimList(&buffList[2], 2);
		{
			vic2[0] = 1000;
			vic2[1] = 2000;
		}
		
		SendBufferRef sendBuffer = pktWriter.CloseAndReturn();

		GSessionManager.Broadcast(sendBuffer);
		this_thread::sleep_for(250ms);
	}

	GThreadManager->Join();
}