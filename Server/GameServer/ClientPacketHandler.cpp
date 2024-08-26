#include "pch.h"
#include "ClientPacketHandler.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

//流立 牧刨明 累诀磊 康开

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	return false;
}

bool Handle_C_TEST(PacketSessionRef& session, Protocol::C_TEST& pkt)
{
	return true;
}

bool Handle_C_MOVE(PacketSessionRef& session, Protocol::C_MOVE& pkt)
{
	return false;
}
