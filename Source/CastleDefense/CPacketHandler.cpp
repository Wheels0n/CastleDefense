#include "CPacketHandler.h"
#include "NetworkWorker.h"
void CPacketHandler::SerializeC_Chat(C_Chat& pkt, char* pBuf)
{
	int packetSize = sizeof(CPacketHeader) + pkt.ByteSizeLong();
	CPacketHeader* pHeader = reinterpret_cast<CPacketHeader*>(pBuf);
	pHeader->size = packetSize;
	pHeader->id = E_TYPE::Chat;

	pkt.SerializeToArray(pHeader + 1, pkt.ByteSizeLong());
}

C_Chat CPacketHandler::ParseC_Chat(char* pBuf)
{
	CPacketHeader* pHeader = reinterpret_cast<CPacketHeader*>(pBuf);
	int size = (pHeader->size) - sizeof(CPacketHeader);
	C_Chat pkt;
	if (pkt.ParseFromArray(pHeader + 1, size) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("ParseFromArray() Failed"));
	}
	else
	{
		std::string msg = pkt.msg();
		FString fstr(msg.c_str());
		GEngine->AddOnScreenDebugMessage(17, 10.0f, FColor::Red, fstr);
	}
	
	return pkt;
}


void CPacketHandler::ProcessPacket(CPacketHeader* pHeader)
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

void CPacketHandler::ProcessC_Chat(CPacketHeader* pHeader)
{
	UE_LOG(LogTemp, Log, TEXT("ProcessC_Chat"));
	ParseC_Chat(reinterpret_cast<char*>(pHeader));
}