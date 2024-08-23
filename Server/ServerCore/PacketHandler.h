#pragma once

#include "test.pb.h"

struct PacketHeader
{
	//ũ��� ���+���̷ε�
	int size;
	int id;
};

class PacketHandler
{
public:
	static void WritePacket(Packet& pkt, char* pBuf);
	static void ReadPacket(char* pBuf);
};

