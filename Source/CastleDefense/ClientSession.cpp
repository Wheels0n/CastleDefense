// Fill out your copyright notice in the Description page of Project Settings.


#include "ClientSession.h"
#include "NetworkWorker.h"
#include "CPacketHandler.h"
#include "Sockets.h"
#include "SendBuffer.h"
void ClientSession::EnqueueRecvPacket(TArray<uint8> packet)
{
	m_recvQueue.Enqueue(packet);
}
void ClientSession::DequeueRecvPacket()
{
	if (!m_recvQueue.IsEmpty())
	{
		UE_LOG(LogTemp, Display, TEXT("DequeueRecvPacket()"));
		TArray<uint8> arr;
		m_recvQueue.Dequeue(arr);
		m_pPacketHandler->ProcessPacket((CPacketHeader*)arr.GetData());
		
	}
}
void ClientSession::EnqueueSendPacket(TSharedPtr<SendBuffer> pkt)
{
	m_sendQueue.Enqueue(pkt);
}
bool ClientSession::DequeueSendPacket(TSharedPtr<SendBuffer>& pPkt)
{
	if (m_sendQueue.IsEmpty())
	{
		return false;
	}
	m_sendQueue.Dequeue(pPkt);
	return true;
}
void ClientSession::CreateWorkers()
{
	m_pRecvWorker = MakeShared<RecvWorker>(m_pSocket, AsShared());
	m_pSendWorker = MakeShared<SendWorker>(m_pSocket, AsShared());
}
void ClientSession::SendC_Login()
{
	C_Login pkt;
	pkt.set_id(m_pSocket->GetPortNo());
	TSharedPtr<SendBuffer> sendbuf = MakeShared<SendBuffer>(sizeof(CPacketHeader) + pkt.ByteSizeLong());
	m_pPacketHandler->SerializeC_Login(pkt, sendbuf->GetBuffer());
	EnqueueSendPacket(sendbuf);
}
void ClientSession::SendC_Spawn()
{
	C_Spawn pkt;
	pkt.set_id(m_pSocket->GetPortNo());
	TSharedPtr<SendBuffer> sendbuf = MakeShared<SendBuffer>(sizeof(CPacketHeader) + pkt.ByteSizeLong());
	m_pPacketHandler->SerializeC_Spawn(pkt, sendbuf->GetBuffer());
	EnqueueSendPacket(sendbuf);
}
void ClientSession::SendC_Despawn()
{
	C_Despawn pkt;
	pkt.set_id(m_pSocket->GetPortNo());
	TSharedPtr<SendBuffer> sendbuf = MakeShared<SendBuffer>(sizeof(CPacketHeader) + pkt.ByteSizeLong());
	m_pPacketHandler->SerializeC_Despawn(pkt, sendbuf->GetBuffer());
	EnqueueSendPacket(sendbuf);
}
void ClientSession::SendC_Move(Coordiante* newPos, Rotation* newRot, MoveState moveState, bool bAttack)
{
	C_Move pkt;
	if (m_player.has_coord() == false)
	{
		m_player.set_allocated_coord(newPos);
		m_player.set_allocated_rot(newRot);
		m_player.set_id(m_pSocket->GetPortNo());
		m_player.set_hp(0);
	}
	m_player.set_battack(bAttack);
	m_player.set_movestate(moveState);
	pkt.set_allocated_player(&m_player);
	
	TSharedPtr<SendBuffer> sendbuf = MakeShared<SendBuffer>(sizeof(CPacketHeader) + pkt.ByteSizeLong());

	m_pPacketHandler->SerializeC_Move(pkt, sendbuf->GetBuffer());
	EnqueueSendPacket(sendbuf);
	
	Player* pPlayer = pkt.release_player();
}
void ClientSession::SendC_Chat(char* pBuf)
{
	C_Chat chat;
	chat.set_msg(pBuf);
	TSharedPtr<SendBuffer> sendbuf = MakeShared<SendBuffer>(sizeof(CPacketHeader) + chat.ByteSizeLong());
	m_pPacketHandler->SerializeC_Chat(chat, sendbuf->GetBuffer());
	EnqueueSendPacket(sendbuf);
}
void ClientSession::SendC_Attack(int idx)
{
	C_Attack pkt;
	pkt.set_attacker(m_pSocket->GetPortNo());
	pkt.set_target(idx);
	TSharedPtr<SendBuffer> sendbuf = MakeShared<SendBuffer>(sizeof(CPacketHeader) + pkt.ByteSizeLong());
	m_pPacketHandler->SerializeC_Attack(pkt, sendbuf->GetBuffer());
	EnqueueSendPacket(sendbuf);
}
ClientSession::ClientSession(FSocket* pSocket, UCastleDefenseGameInstance* pGameInstance)
	:m_pSocket(pSocket), m_pSendWorker(nullptr), m_pRecvWorker(nullptr)
{
	m_pPacketHandler = MakeShared<CPacketHandler>(pGameInstance, m_pSocket->GetPortNo());
}

ClientSession::~ClientSession()
{
	m_pSendWorker = nullptr;
	m_pRecvWorker = nullptr;
	m_pPacketHandler = nullptr;
}
