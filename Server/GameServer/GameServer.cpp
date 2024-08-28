#include "pch.h"
#include "ThreadManager.h"
#include "Service.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "BufferWriter.h"
#include "ClientPacketHandler.h"
#include "Protocol.pb.h"
#include "job.h"

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
	//TEST JOB
	{
		//[일감 의뢰 내용] : 1번 유저한테 10만큼 힐을 줘라!.

		//행동 : Heal
		//인자 : 1번유저, 10이라는 힐량

		HealJob healJob;
		healJob._target = 1;
		healJob._healIValue = 10;

		//나중에
		healJob.Execute();
	}

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