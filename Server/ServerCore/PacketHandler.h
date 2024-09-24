#pragma once

#include "test.pb.h"

struct PacketHeader
{
	//ũ��� ���+���̷ε�
	int size;
	int id;
};

class Session;
class PacketHandler
{
public:
	static void SerializeS_Login(S_Login& pkt, char* pBuf);
	static void SerializeS_Spawn(S_Spawn& pkt, char* pBuf);
	static void SerializeS_Despawn(S_Despawn& pkt, char* pBuf);
	static void SerializeS_Move(S_Move& pkt, char* pBuf);
	static void SerializeS_Chat(S_Chat& pkt, char* pBuf);
	
	static void ProcessPacket(PacketHeader*, shared_ptr<Session>);

	static void ProcessC_Login(PacketHeader*, shared_ptr<Session>);
	static void ProcessC_Spawn(PacketHeader*);
	static void ProcessC_Despawn(PacketHeader*);
	static void ProcessC_Move(PacketHeader*);
	static void ProcessC_Chat(PacketHeader*, shared_ptr<Session> );

	static C_Chat ParseC_Chat(char* pBuf);
	static C_Login ParseC_Login(char* pBuf);
	static C_Spawn ParseC_Spawn(char* pBuf);
	static C_Despawn ParseC_Despawn(char* pBuf);
	static C_Move ParseC_Move(char* pBuf);
};

