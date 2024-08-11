#include "pch.h"
#include "ClientPacketHandler.h"
#include "BufferReader.h"

void ClientPacketHandler::HandlePacket(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);
	PacketHeader header;
	br >> header;


	switch (header.id)
	{
	case S_TEST:
		Handler_S_TEST(buffer,len);
		break;

	}
}

struct BuffData
{
	uint64 buffId;
	float remainTime;
};

struct S_TEST
{
	uint64 id;
	uint32 hp;
	uint16 attack;
	//가변데이터
	//1. 문자열
	//2. 바이트배열
	//3. 일반 리스트

	vector<BuffData> buffs;
	wstring name;
	
};

void ClientPacketHandler::Handler_S_TEST(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);
	PacketHeader header;
	br >> header;

	uint64 id;
	uint32 hp;
	uint16 attack;
	br >> id >> hp >> attack;

	cout << "ID :" << id << "HP : " << hp << "ATTCK : " << attack << '\n';

	vector<BuffData> buffs;
	uint16 buffCount;
	br >> buffCount;

	buffs.resize(buffCount);
	for (int32 i = 0; i < buffCount; i++)
	{
		br >> buffs[i].buffId >> buffs[i].remainTime;
	}

	cout << "BuffCount" << buffCount << '\n';

	wstring name;
	uint16 nameLen;
	br >> nameLen;
	name.resize(nameLen);
	br.Read((void*)name.data(), nameLen * sizeof(WCHAR));

	wcout.imbue(std::locale("kor"));
	wcout << name << '\n';
}
