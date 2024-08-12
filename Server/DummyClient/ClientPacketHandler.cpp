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
		
		size += buffsCount * sizeof(BuffsListItem);
		if (size != packetSize)
			return false;

		if (buffoffset + buffsCount * sizeof(BuffsListItem) > packetSize)
			return false;

		return true;
	}

	using BuffList = PacketList<PKT_S_TEST::BuffsListItem>;
	BuffList GetBuffsList()
	{
		BYTE* data = reinterpret_cast<BYTE*>(this);
		data += buffoffset;
		return BuffList(reinterpret_cast<PKT_S_TEST::BuffsListItem*>(data), buffsCount);
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

	if (len < sizeof(PKT_S_TEST))
		return;

	PKT_S_TEST* pkt = reinterpret_cast<PKT_S_TEST*>(buffer);

	if (pkt->Validate() == false)
		return;
	//cout << "ID :" << id << "HP : " << hp << "ATTCK : " << attack << '\n';
	
	PKT_S_TEST::BuffList buffs = pkt->GetBuffsList();

	cout << "BuffCount" << buffs.Count()<< '\n';

	for (int32 i = 0; i < buffs.Count(); i++)
	{
		cout << "Bufinfo : " << buffs[i].buffId << " " << buffs[i].remainTime << '\n';
	}

	for (auto it = buffs.begin(); it != buffs.end(); ++it)
	{
		cout << it->buffId << " " << it->remainTime << '\n';
	}

	for (auto& buff : buffs)
	{
		cout << buff.buffId << " " << buff.remainTime << '\n';
	}
}
