#include "pch.h"
#include "ClientPacketHandler.h"
#include "BufferReader.h"
#include "Protocol.pb.h"

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


void ClientPacketHandler::Handler_S_TEST(BYTE* buffer, int32 len)
{
	Protocol::S_TEST pkt;

	ASSERT_CRASH(pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)));
	cout << pkt.id() << " " << pkt.hp() << " " << pkt.attack() << '\n';
	cout << "BUFSIZE: " << pkt.buffs_size() << '\n';

	for (auto& buf : pkt.buffs())
	{
		cout << "BUFINFO: " << buf.buffid() << " " << buf.remaintime() << '\n';
		cout << "VICTIMES: " << buf.victims_size() << '\n';
		for (auto& vic : buf.victims())
		{
			cout << vic<<" ";
		}
		cout << '\n';
	}
}
