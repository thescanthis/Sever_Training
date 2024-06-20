#pragma once
#include "NetAddress.h"

/*------------------
	SocketUtils
------------------*/

class SocketUtils
{
public:
	//실질적으로 런타임에 긁어와야하는 값들임
	static LPFN_CONNECTEX		ConnectEx;
	static LPFN_DISCONNECTEX	DisconnectEx;
	static LPFN_ACCEPTEX		AcceptEx;

public:

	static void Init();
	static void Clear();

	static bool BindWindowsFunton(SOCKET socket, GUID guid, LPVOID* fn);
	static SOCKET CreateSocket();
};

