#include "stdafx.h"
#include "PacketHandler.h"
#include "Session.h"
#include "SessionManager.h"
void PacketHandler::SerializeC_Chat(C_Chat& pkt, char* pBuf)
{
	int packetSize = sizeof(PacketHeader) + pkt.ByteSizeLong();
	PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(pBuf);
	pHeader->size = packetSize;
	pHeader->id = E_TYPE::Chat;
	
	pkt.SerializeToArray(pHeader+1, pkt.ByteSizeLong());
}

C_Chat PacketHandler::ParseC_Chat(char* pBuf)
{
	PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(pBuf);
	int size = (pHeader->size) - sizeof(PacketHeader);
	C_Chat pkt;
	pkt.ParseFromArray(pHeader+1, size);
	cout << pkt.msg() << endl;
	return pkt;
}

void PacketHandler::ProcessPacket(PacketHeader* pHeader)
{
	E_TYPE id = (E_TYPE)pHeader->id;
	switch (id)
	{
	case Chat:
		ProcessC_Chat(pHeader);
		break;
	default:
		break;
	}

}

void PacketHandler::ProcessC_Chat(PacketHeader* pHeader)
{	
	int packetSize = pHeader->size;
	ParseC_Chat((char*)pHeader);
	shared_ptr<SendBuffer> pSendBuffer = make_shared<SendBuffer>(packetSize);
	memcpy(pSendBuffer->GetBuffer(),pHeader, packetSize);
	g_pSessionManager->Brodcast(pSendBuffer);
}
