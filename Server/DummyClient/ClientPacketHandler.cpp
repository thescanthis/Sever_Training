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

//struct를 보낼때 데이터 크기를 있는 그대로보냄
//원
#pragma pack(1)
//[PKT_S_TEST][BuffsListItem BuffsListItem BuffsListItem]
struct PKT_S_TEST
{
	struct BuffsListItem
	{
		uint64 buffId;
		float remainTime;

		uint16 victimsOffset;
		uint16 victimsCount;

		bool Validata(BYTE* packetstart, uint16 packetSize, OUT uint32& size)
		{
			if (victimsOffset + victimsCount * sizeof(uint64) > packetSize)
				return false;

			size += victimsCount * sizeof(uint64);
			return true;
		}
	};

	uint16 packetSize;
	uint16 packetid;
	uint64 id;
	uint32 hp;
	uint16 attack;

	uint16 buffoffset;
	uint16 buffsCount;

	bool Validate()
	{
		uint32 size = 0;
		size += sizeof(PKT_S_TEST);
		if (packetSize < size)
			return false;
		
		if (buffoffset + buffsCount * sizeof(BuffsListItem) > packetSize)
			return false;

		size += buffsCount * sizeof(BuffsListItem);

		BuffsList buffList = GetBuffsList();

		for (int32 i = 0; i < buffList.Count(); i++)
		{
			if (buffList[i].Validata((BYTE*)this, packetSize, OUT size) == false)
				return false;
		}

		//최종크기 비교
		if (size != packetSize)
			return false;

		return true;
	}

	using BuffsList = PacketList<PKT_S_TEST::BuffsListItem>;
	using BuffsVictimsList = PacketList<uint64>;

	BuffsList GetBuffsList()
	{
		BYTE* data = reinterpret_cast<BYTE*>(this);
		data += buffoffset;
		return BuffsList(reinterpret_cast<PKT_S_TEST::BuffsListItem*>(data), buffsCount);
	}

	BuffsVictimsList GetBuffsVictimeList(BuffsListItem* buffsItem)
	{
		BYTE* data = reinterpret_cast<BYTE*>(this);
		data += buffsItem->victimsOffset;
		return BuffsVictimsList(reinterpret_cast<uint64*>(data), buffsItem->victimsCount);
	}
	//가변데이터
	//1. 문자열
	//2. 바이트배열
	//3. 일반 리스트

	//vector<BuffData> buffs;
	//wstring name;
	
};
#pragma pack()

void ClientPacketHandler::Handler_S_TEST(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	PKT_S_TEST* pkt = reinterpret_cast<PKT_S_TEST*>(buffer);

	if (pkt->Validate() == false)
		return;
	//cout << "ID :" << id << "HP : " << hp << "ATTCK : " << attack << '\n';
	
	PKT_S_TEST::BuffsList buffs = pkt->GetBuffsList();

	cout << "BuffCount" << buffs.Count()<< '\n';

	for (auto& buff : buffs)
	{
		cout << buff.buffId << " " << buff.remainTime << '\n';

		PKT_S_TEST::BuffsVictimsList victimes =  pkt->GetBuffsVictimeList(&buff);

		cout << "Victim Count" << victimes.Count() << '\n';

		for (auto& victim : victimes)
		{
			cout << "Victimes : " << victim << '\n';
		}
	}
}
