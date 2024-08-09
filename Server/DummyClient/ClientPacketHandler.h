#pragma once

enum {
	S_TEST=1
};

class ClientPacketHandler
{
public:
	static void HandlePacket(BYTE* buffer, int32 len);

	static void Handler_S_TEST(BYTE* buffer, int32 len);
};

