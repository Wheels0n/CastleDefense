#pragma once

#include "test.pb.h"

struct PacketHeader
{
	//크기는 헤더+페이로드
	int size;
	int id;
};

class PacketHandler
{
public:
	static void SerializeC_Chat(C_Chat& pkt, char* pBuf);
	static C_Chat ParseC_Chat(char* pBuf);
	static void ProcessPacket(PacketHeader*);

	static void ProcessC_Chat(PacketHeader*);
};

