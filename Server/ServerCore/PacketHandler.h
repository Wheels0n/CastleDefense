#pragma once

#include "test.pb.h"

struct PacketHeader
{
	//크기는 헤더+페이로드
	int size;
	int id;
};

class Session;
class PacketHandler
{
public:
	static void			SerializeS_Login(S_Login& pkt, char* pBuf);
	static void			SerializeS_Spawn(S_Spawn& pkt, char* pBuf);
	static void			SerializeS_Despawn(S_Despawn& pkt, char* pBuf);
	static void			SerializeS_Move(S_Move& pkt, char* pBuf);
	static void			SerializeS_Chat(S_Chat& pkt, char* pBuf);
	static void			SerializeS_Attack(S_Attack& pkt, char* pBuf);
	static void			SerializeS_EnemySpawn(S_EnemySpawn& pkt, char* pBuf);
	static void			SerializeS_EnemyMovement(S_EnemyMove& pkt, char* pBuf);
	
	static void			ProcessPacket(PacketHeader*, shared_ptr<Session>);

	static void			ProcessC_Login(PacketHeader*, shared_ptr<Session>);
	static void			ProcessC_Spawn(PacketHeader*);
	static void			ProcessC_Despawn(PacketHeader*);
	static void			ProcessC_Move(PacketHeader*);
	static void			ProcessC_Chat(PacketHeader*, shared_ptr<Session> );
	static void			ProcessC_Attack(PacketHeader*);

	static C_Login		ParseC_Login(char* pBuf);
	static C_Spawn		ParseC_Spawn(char* pBuf);
	static C_Despawn	ParseC_Despawn(char* pBuf);
	static C_Move		ParseC_Move(char* pBuf);
	static C_Chat		ParseC_Chat(char* pBuf);
	static C_Attack		ParseC_Attack(char* pBuf);

	static void			BrodcastS_Move();
	static void			BrodcastS_EnemyMove();
	
};

