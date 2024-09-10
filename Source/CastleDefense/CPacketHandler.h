#pragma once
#include "CoreMinimal.h"
#include "Network/test.pb.h"
struct CPacketHeader;
class CPacketHandler
{
public:
	static void SerializeC_Chat(C_Chat& pkt, char* pBuf);
	static C_Chat ParseC_Chat(char* pBuf);
	static void ProcessPacket(CPacketHeader*);

	static void ProcessC_Chat(CPacketHeader*);
};
