#pragma once
#include "CoreMinimal.h"
#include "Network/test.pb.h"
struct CPacketHeader;
class UCastleDefenseGameInstance;

class CPacketHandler : public TSharedFromThis<CPacketHandler>
{
public:
	void SerializeC_Login(C_Login& pkt, char* pBuf);
	void SerializeC_Spawn(C_Spawn& pkt, char* pBuf);
	void SerializeC_Despawn(C_Despawn& pkt, char* pBuf);
	void SerializeC_Move(C_Move& pkt, char* pBuf);
	void SerializeC_Chat(C_Chat& pkt, char* pBuf);
	void SerializeC_Attack(C_Attack& pkt, char* pBuf);

	S_Login ParseS_Login(char* pBuf);
	S_Spawn ParseS_Spawn(char* pBuf);
	S_Despawn ParseS_Despawn(char* pBuf);
	S_Move ParseS_Move(char* pBuf);
	S_Chat ParseS_Chat(char* pBuf);
	S_EnemySpawn ParseS_EnemySpawn(char* pBuf);
	S_Attack ParseS_Attack(char* pBuf);
	S_EnemyMove ParseS_EnemyMove(char* pBuf);

	void ProcessPacket(CPacketHeader*);

	void ProcessS_Login(CPacketHeader*);
	void ProcessS_Spawn(CPacketHeader*);
	void ProcessS_Despawn(CPacketHeader*);
	void ProcessS_Move(CPacketHeader*);
	void ProcessS_Chat(CPacketHeader*);
	void ProcessS_EnemySpawn(CPacketHeader*);
	void ProcessS_Attack(CPacketHeader*);
	void ProcessS_EnemyMove(CPacketHeader*);

	CPacketHandler(UCastleDefenseGameInstance*, int);

private:
	UCastleDefenseGameInstance* m_pGameInstance;
	int m_id;
};
