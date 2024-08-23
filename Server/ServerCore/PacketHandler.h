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
	static void WritePacket(Packet& pkt, char* pBuf);
	static void ReadPacket(char* pBuf);
};

