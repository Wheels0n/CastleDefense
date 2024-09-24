#include "CPacketHandler.h"
#include "NetworkWorker.h"
#include "CastleDefenseGameInstance.h"
#include "CastleDefenseGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Wizard.h"
void CPacketHandler::SerializeC_Login(C_Login& pkt, char* pBuf)
{
	int packetSize = sizeof(CPacketHeader) + pkt.ByteSizeLong();
	CPacketHeader* pHeader = reinterpret_cast<CPacketHeader*>(pBuf);
	pHeader->size = packetSize;
	pHeader->id = E_TYPE::Login;

	pkt.SerializeToArray(pHeader + 1, pkt.ByteSizeLong());
}
void CPacketHandler::SerializeC_Spawn(C_Spawn& pkt, char* pBuf)
{
	int packetSize = sizeof(CPacketHeader) + pkt.ByteSizeLong();
	CPacketHeader* pHeader = reinterpret_cast<CPacketHeader*>(pBuf);
	pHeader->size = packetSize;
	pHeader->id = E_TYPE::Spawn;

	pkt.SerializeToArray(pHeader + 1, pkt.ByteSizeLong());
}
void CPacketHandler::SerializeC_Despawn(C_Despawn& pkt, char* pBuf)
{
	int packetSize = sizeof(CPacketHeader) + pkt.ByteSizeLong();
	CPacketHeader* pHeader = reinterpret_cast<CPacketHeader*>(pBuf);
	pHeader->size = packetSize;
	pHeader->id = E_TYPE::Despawn;

	pkt.SerializeToArray(pHeader + 1, pkt.ByteSizeLong());
}
void CPacketHandler::SerializeC_Move(C_Move& pkt, char* pBuf)
{
	int packetSize = sizeof(CPacketHeader) + pkt.ByteSizeLong();
	CPacketHeader* pHeader = reinterpret_cast<CPacketHeader*>(pBuf);
	pHeader->size = packetSize;
	pHeader->id = E_TYPE::Movement;

	pkt.SerializeToArray(pHeader + 1, pkt.ByteSizeLong());
}
void CPacketHandler::SerializeC_Chat(C_Chat& pkt, char* pBuf)
{
	int packetSize = sizeof(CPacketHeader) + pkt.ByteSizeLong();
	CPacketHeader* pHeader = reinterpret_cast<CPacketHeader*>(pBuf);
	pHeader->size = packetSize;
	pHeader->id = E_TYPE::Chat;

	pkt.SerializeToArray(pHeader + 1, pkt.ByteSizeLong());
}

S_Login CPacketHandler::ParseS_Login(char* pBuf)
{
	CPacketHeader* pHeader = reinterpret_cast<CPacketHeader*>(pBuf);
	int size = (pHeader->size) - sizeof(CPacketHeader);
	S_Login pkt;
	if (pkt.ParseFromArray(pHeader + 1, size) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("ParseFromArray() Failed"));
	}
	return pkt;
}

S_Spawn CPacketHandler::ParseS_Spawn(char* pBuf)
{
	CPacketHeader* pHeader = reinterpret_cast<CPacketHeader*>(pBuf);
	int size = (pHeader->size) - sizeof(CPacketHeader);
	S_Spawn pkt;
	if (pkt.ParseFromArray(pHeader + 1, size) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("ParseFromArray() Failed"));
	}
	return pkt;
}

S_Despawn CPacketHandler::ParseS_Despawn(char* pBuf)
{
	CPacketHeader* pHeader = reinterpret_cast<CPacketHeader*>(pBuf);
	int size = (pHeader->size) - sizeof(CPacketHeader);
	S_Despawn pkt;
	if (pkt.ParseFromArray(pHeader + 1, size) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("ParseFromArray() Failed"));
	}
	return pkt;
}

S_Move CPacketHandler::ParseS_Move(char* pBuf)
{
	CPacketHeader* pHeader = reinterpret_cast<CPacketHeader*>(pBuf);
	int size = (pHeader->size) - sizeof(CPacketHeader);
	S_Move pkt;
	if (pkt.ParseFromArray(pHeader + 1, size) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("ParseFromArray() Failed"));
	}
	return pkt;
}

S_Chat CPacketHandler::ParseS_Chat(char* pBuf)
{
	CPacketHeader* pHeader = reinterpret_cast<CPacketHeader*>(pBuf);
	int size = (pHeader->size) - sizeof(CPacketHeader);
	S_Chat pkt;
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
	case Login:
		ProcessS_Login(pHeader);
		break;
	case Spawn:
		ProcessS_Spawn(pHeader);
		break;
	case Despawn:
		ProcessS_Despawn(pHeader);
		break;
	case Movement:
		ProcessS_Move(pHeader);
		break;
	case Chat:
		ProcessS_Chat(pHeader);
		break;
	default:
		break;
	}

}

void CPacketHandler::ProcessS_Login(CPacketHeader* pHeader)
{
	S_Login pkt =ParseS_Login(reinterpret_cast<char*>(pHeader));
	if (pkt.bsucceded())
	{
		UE_LOG(LogTemp, Log, TEXT("LoginSucceded"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("LoginFailed"));
	}

}

void CPacketHandler::ProcessS_Spawn(CPacketHeader* pHeader)
{
	S_Spawn pkt = ParseS_Spawn(reinterpret_cast<char*>(pHeader));

	GEngine->AddOnScreenDebugMessage(INDEX_NONE, 13.0f, FColor::Yellow, FString::Printf(TEXT("new players : %d"), pkt.player_size()));
	UWorld* pCurWorld = m_pGameInstance->GetWorld();
	ACastleDefenseGameState* pGameState = pCurWorld->GetGameState<ACastleDefenseGameState>();
	for (int i = 0; i < pkt.player_size(); ++i)
	{
		Player player = pkt.player(i);
		if (pGameState->GetPlayerById(player.id()) 
			== nullptr)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 23.0f, FColor::Yellow, FString::Printf(TEXT("new player ID : %d"), player.id()));
			pGameState->AddPlayer(&player, m_id);
		}

	}
}

void CPacketHandler::ProcessS_Despawn(CPacketHeader* pHeader)
{
	S_Despawn pkt = ParseS_Despawn(reinterpret_cast<char*>(pHeader));
	UWorld* pCurWorld = m_pGameInstance->GetWorld();
	ACastleDefenseGameState* pGameState = pCurWorld->GetGameState<ACastleDefenseGameState>();
	pGameState->RemovePlayerById(pkt.id());
	//TODO: 게임모드에서 캐릭터 죽으면 처리
}

void CPacketHandler::ProcessS_Move(CPacketHeader* pHeader)
{
	S_Move pkt = ParseS_Move(reinterpret_cast<char*>(pHeader));
	UWorld* pCurWorld = m_pGameInstance->GetWorld();
	ACastleDefenseGameState* pGameState = pCurWorld->GetGameState<ACastleDefenseGameState>();
	Player player = pkt.player();
	//늦게 접속한 경우 대비
	if (pGameState->GetPlayerById(player.id())!=nullptr)
	{
		pGameState->UpdatePlayerPos(&player);
	}
	
}

void CPacketHandler::ProcessS_Chat(CPacketHeader* pHeader)
{
	UE_LOG(LogTemp, Log, TEXT("ProcessC_Chat"));
	S_Chat pkt = ParseS_Chat(reinterpret_cast<char*>(pHeader));
	
	UWorld* pCurWorld = m_pGameInstance->GetWorld();
	ACastleDefenseGameState* pGameState = pCurWorld->GetGameState<ACastleDefenseGameState>();
	AWizard* pPlayer =  pGameState->GetPlayerById(m_id);
	std::string msg = pkt.msg();
	pPlayer->AddChat(msg);
	
}

CPacketHandler::CPacketHandler(UCastleDefenseGameInstance* pGameInstacne, int id)
	:m_pGameInstance(pGameInstacne),m_id(id)
{

}
