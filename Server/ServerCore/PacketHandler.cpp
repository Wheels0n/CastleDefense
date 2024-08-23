#include "stdafx.h"
#include "PacketHandler.h"

void PacketHandler::WritePacket(Packet& pkt, char* pBuf)
{
	int packetSize = sizeof(PacketHeader) + pkt.ByteSizeLong();
	PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(pBuf);
	pHeader->size = packetSize;
	pHeader->id = pkt.id();
	
	pkt.SerializeToArray(pHeader+1, pkt.ByteSizeLong());
}

void PacketHandler::ReadPacket(char* pBuf)
{
	PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(pBuf);
	Packet pkt;
	int size = (pHeader->size) - sizeof(PacketHeader);
	pkt.ParseFromArray(pHeader+1, size);

	cout << pkt.id() <<" " << pkt.msg() << endl;
}
