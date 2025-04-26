#include "stdafx.h"
#include "PacketHandler.h"
#include "Session.h"
#include "SessionManager.h"
#include "PlayerManager.h"
#include "EnemyManager.h"

void		PacketHandler::SerializeS_Login(S_Login& pkt, char* pBuf)
{
	int packetSize = sizeof(PacketHeader) + pkt.ByteSizeLong();
	PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(pBuf);
	pHeader->size = packetSize;
	pHeader->id = E_TYPE::Login;

	pkt.SerializeToArray(pHeader + 1, pkt.ByteSizeLong());
}
void		PacketHandler::SerializeS_Spawn(S_Spawn& pkt, char* pBuf)
{
	int packetSize = sizeof(PacketHeader) + pkt.ByteSizeLong();
	PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(pBuf);
	pHeader->size = packetSize;
	pHeader->id = E_TYPE::Spawn;

	pkt.SerializeToArray(pHeader + 1, pkt.ByteSizeLong());
}
void		PacketHandler::SerializeS_Despawn(S_Despawn& pkt, char* pBuf)
{
	int packetSize = sizeof(PacketHeader) + pkt.ByteSizeLong();
	PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(pBuf);
	pHeader->size = packetSize;
	pHeader->id = E_TYPE::Despawn;

	pkt.SerializeToArray(pHeader + 1, pkt.ByteSizeLong());
}
void		PacketHandler::SerializeS_Move(S_Move& pkt, char* pBuf)
{
	int packetSize = sizeof(PacketHeader) + pkt.ByteSizeLong();
	PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(pBuf);
	pHeader->size = packetSize;
	pHeader->id = E_TYPE::Movement;

	pkt.SerializeToArray(pHeader + 1, pkt.ByteSizeLong());
}
void		PacketHandler::SerializeS_Chat(S_Chat& pkt, char* pBuf)
{
	int packetSize = sizeof(PacketHeader) + pkt.ByteSizeLong();
	PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(pBuf);
	pHeader->size = packetSize;
	pHeader->id = E_TYPE::Chat;
	
	pkt.SerializeToArray(pHeader+1, pkt.ByteSizeLong());
}
void		PacketHandler::SerializeS_Attack(S_Attack& pkt, char* pBuf)
{
	int packetSize = sizeof(PacketHeader) + pkt.ByteSizeLong();
	PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(pBuf);
	pHeader->size = packetSize;
	pHeader->id = E_TYPE::Attack;

	pkt.SerializeToArray(pHeader + 1, pkt.ByteSizeLong());
}
void		PacketHandler::SerializeS_EnemySpawn(S_EnemySpawn& pkt, char* pBuf)
{
	int packetSize = sizeof(PacketHeader) + pkt.ByteSizeLong();
	PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(pBuf);
	pHeader->size = packetSize;
	pHeader->id = E_TYPE::EnemySpawn;

	pkt.SerializeToArray(pHeader + 1, pkt.ByteSizeLong());
}
void		PacketHandler::SerializeS_EnemyMovement(S_EnemyMove& pkt, char* pBuf)
{
	int packetSize = sizeof(PacketHeader) + pkt.ByteSizeLong();
	PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(pBuf);
	pHeader->size = packetSize;
	pHeader->id = E_TYPE::EnemyMovement;

	pkt.SerializeToArray(pHeader + 1, pkt.ByteSizeLong());
}

void		PacketHandler::ProcessPacket(PacketHeader* pHeader, shared_ptr<Session> pSession)
{
	E_TYPE id = (E_TYPE)pHeader->id;
	switch (id)
	{
	case Login:
		ProcessC_Login(pHeader, pSession);
		break;
	case Spawn:
		ProcessC_Spawn(pHeader);
		break;
	case Despawn:
		ProcessC_Despawn(pHeader);
		break;
	case Movement:
		ProcessC_Move(pHeader);
		break;
	case Chat:
		ProcessC_Chat(pHeader, pSession);
		break;
	case Attack:
		ProcessC_Attack(pHeader);
		break;
	default:
		cout << "Packet Header Error!" << "\n";
		break;
	}

}

void		PacketHandler::ProcessC_Login(PacketHeader* pHeader, shared_ptr<Session> pSession)
{
	C_Login c_login = ParseC_Login(reinterpret_cast<char*>(pHeader));
	SessionManager::GetInstance().MapIdToSession(c_login.id(), pSession);

	S_Login pkt;
	pkt.set_bsucceded(true);
	int packetSize = sizeof(PacketHeader) + pkt.ByteSizeLong();
	shared_ptr<SendBuffer> pSendBuffer = make_shared<SendBuffer>(packetSize);
	SerializeS_Login(pkt, pSendBuffer->GetBuffer());
	pSession->RequestSend(pSendBuffer);

}
void		PacketHandler::ProcessC_Spawn(PacketHeader* pHeader)
{
	C_Spawn c_spawn = ParseC_Spawn(reinterpret_cast<char*>(pHeader));
	PlayerManager::GetInstance().AddPlayerById(c_spawn.id());

	//기존 플레이어들에게 먼저 브로드 캐스팅
	S_Spawn pkt;
	shared_ptr<PlayerInfo> pSharedPlayer	= PlayerManager::GetInstance().GetPlayerById(c_spawn.id());
	PlayerInfo*					pPlayer		= pSharedPlayer.get();
	Player*						pCurPlayer	= pkt.add_player();
	*pCurPlayer = *pPlayer->GetPlayer();

	shared_ptr<Session> pCurSession = SessionManager::GetInstance().GetSessionById(c_spawn.id());


	int packetSize = sizeof(PacketHeader) + pkt.ByteSizeLong();
	shared_ptr<SendBuffer> pSendBuffer = make_shared<SendBuffer>(packetSize);
	SerializeS_Spawn(pkt, pSendBuffer->GetBuffer());

	SessionManager::GetInstance().Brodcast(pSendBuffer, pCurSession);

	//기존 플레이어들도 직렬화 해서 새 플레이어 send
	{
		S_Spawn pkt;
		xmap<int, shared_ptr<PlayerInfo>>& idToPlayer = PlayerManager::GetInstance().GetIdToPlayerMap();
		for (auto it = idToPlayer.begin(); it != idToPlayer.end(); ++it)
		{
			{
				Player* pCurPlayer = pkt.add_player();
				shared_ptr<PlayerInfo> pSharedPlayer = it->second;
				*pCurPlayer = *pSharedPlayer->GetPlayer();

			}
		}
		int packetSize = sizeof(PacketHeader) + pkt.ByteSizeLong();
		shared_ptr<SendBuffer> pSendBuffer = make_shared<SendBuffer>(packetSize);
		SerializeS_Spawn(pkt, pSendBuffer->GetBuffer());
		shared_ptr<Session> pSession = SessionManager::GetInstance().GetSessionById(c_spawn.id());
		pSession->RequestSend(pSendBuffer);
	}

	//적 스폰 패킷 
	{
		S_EnemySpawn pkt;
		EnemyManager::GetInstance().AddEnemyToSpawnPacket(pkt);
		int packetSize = sizeof(PacketHeader) + pkt.ByteSizeLong();
		shared_ptr<SendBuffer> pSendBuffer = make_shared<SendBuffer>(packetSize);
		PacketHandler::SerializeS_EnemySpawn(pkt, pSendBuffer->GetBuffer());
		shared_ptr<Session> pSession = SessionManager::GetInstance().GetSessionById(c_spawn.id());
		pSession->RequestSend(pSendBuffer);
	}
}
void		PacketHandler::ProcessC_Despawn(PacketHeader* pHeader)
{
	C_Despawn c_despawn = ParseC_Despawn(reinterpret_cast<char*>(pHeader));
	PlayerManager::GetInstance().RemovePlayerById(c_despawn.id());

	S_Despawn pkt;
	pkt.set_id(c_despawn.id());
	int packetSize = sizeof(PacketHeader) + pkt.ByteSizeLong();
	shared_ptr<SendBuffer> pSendBuffer = make_shared<SendBuffer>(packetSize);
	SerializeS_Despawn(pkt, pSendBuffer->GetBuffer());

	shared_ptr<Session> pCurSession = SessionManager::GetInstance().GetSessionById(c_despawn.id());
	SessionManager::GetInstance().Brodcast(pSendBuffer, pCurSession);
}
void		PacketHandler::ProcessC_Move(PacketHeader* pHeader)
{
	C_Move c_move = ParseC_Move(reinterpret_cast<char*>(pHeader));
	const Player& playerRef = c_move.player();
	PlayerManager::GetInstance().UpdatePlayer(playerRef);
}
void		PacketHandler::ProcessC_Chat(PacketHeader* pHeader, shared_ptr<Session> pSession)
{
	int packetSize = pHeader->size;
	C_Chat chat = ParseC_Chat((char*)pHeader);
	S_Chat pkt;
	pkt.set_msg(chat.msg());
	shared_ptr<SendBuffer> pSendBuffer = make_shared<SendBuffer>(packetSize);
	SerializeS_Chat(pkt, pSendBuffer->GetBuffer());
	SessionManager::GetInstance().Brodcast(pSendBuffer, pSession);

}
void		PacketHandler::ProcessC_Attack(PacketHeader* pHeader)
{
	C_Attack pkt = ParseC_Attack((char*)pHeader);

	//서버 판정
	shared_ptr<PlayerInfo> pPlayerInfo = PlayerManager::GetInstance().GetPlayerById(pkt.attacker());
	Coordiante* pPlayerCoord = pPlayerInfo->GetCoord();
	shared_ptr<EnemyInfo> pEnemyInfo = EnemyManager::GetInstance().GetEnemyById(pkt.target());
	Coordiante* pEnemyCoord = pEnemyInfo->GetCoord();
	int xLen = abs(pEnemyCoord->x() - pPlayerCoord->x());
	int yLen = abs(pEnemyCoord->y() - pPlayerCoord->y());

	bool bInRange = sqrt(xLen * xLen + yLen * yLen) < 510;
	if (bInRange)
	{
		EnemyManager::GetInstance().DecreaseHp(pkt.target());
		cout << pkt.attacker() << " attack " << pkt.target() << "\n";
		S_Attack response;
		response.set_target(pkt.target());

		int packetSize = sizeof(PacketHeader) + response.ByteSizeLong();
		shared_ptr<SendBuffer> pSendBuffer = make_shared<SendBuffer>(packetSize);
		SerializeS_Attack(response, pSendBuffer->GetBuffer());
		SessionManager::GetInstance().Brodcast(pSendBuffer, nullptr);

	}

}

C_Login		PacketHandler::ParseC_Login(char* pBuf)
{
	PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(pBuf);
	int size = (pHeader->size) - sizeof(PacketHeader);
	C_Login pkt;
	pkt.ParseFromArray(pHeader + 1, size);
	return pkt;
}
C_Spawn		PacketHandler::ParseC_Spawn(char* pBuf)
{
	PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(pBuf);
	int size = (pHeader->size) - sizeof(PacketHeader);
	C_Spawn pkt;
	pkt.ParseFromArray(pHeader + 1, size);
	return pkt;
}
C_Despawn	PacketHandler::ParseC_Despawn(char* pBuf)
{
	PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(pBuf);
	int size = (pHeader->size) - sizeof(PacketHeader);
	C_Despawn pkt;
	pkt.ParseFromArray(pHeader + 1, size);
	return pkt;
}
C_Move		PacketHandler::ParseC_Move(char* pBuf)
{
	PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(pBuf);
	int size = (pHeader->size) - sizeof(PacketHeader);
	C_Move pkt;
	pkt.ParseFromArray(pHeader + 1, size);
	return pkt;
}
C_Chat		PacketHandler::ParseC_Chat(char* pBuf)
{
	PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(pBuf);
	int size = (pHeader->size) - sizeof(PacketHeader);
	C_Chat pkt;
	pkt.ParseFromArray(pHeader + 1, size);
	cout << pkt.msg() << "\n";
	return pkt;
}
C_Attack	PacketHandler::ParseC_Attack(char* pBuf)
{
	PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(pBuf);
	int size = (pHeader->size) - sizeof(PacketHeader);
	C_Attack pkt;
	pkt.ParseFromArray(pHeader + 1, size);
	return pkt;
}

void		PacketHandler::BrodcastS_Move()
{
	S_Move pkt;
	PlayerManager::GetInstance().AddPlayerToMovePacket(pkt);

	int packetSize = sizeof(PacketHeader) + pkt.ByteSizeLong();
	shared_ptr<SendBuffer> pSendBuffer = make_shared<SendBuffer>(packetSize);
	PacketHandler::SerializeS_Move(pkt, pSendBuffer->GetBuffer());
	SessionManager::GetInstance().Brodcast(pSendBuffer, nullptr);
}
void		PacketHandler::BrodcastS_EnemyMove()
{
	S_EnemyMove pkt;
	EnemyManager::GetInstance().AddEnemyToMovePacket(pkt);

	int packetSize = sizeof(PacketHeader) + pkt.ByteSizeLong();
	shared_ptr<SendBuffer> pSendBuffer = make_shared<SendBuffer>(packetSize);
	PacketHandler::SerializeS_EnemyMovement(pkt, pSendBuffer->GetBuffer());
	SessionManager::GetInstance().Brodcast(pSendBuffer, nullptr);
}


