// Fill out your copyright notice in the Description page of Project Settings.


#include "ClientSession.h"
#include "NetworkWorker.h"
#include "CPacketHandler.h"
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
		CPacketHandler::ProcessPacket((CPacketHeader*)arr.GetData());
		
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
void ClientSession::SendC_Chat(char* pBuf)
{
	C_Chat chat;
	chat.set_msg("Hello");
	TSharedPtr<SendBuffer> sendbuf = MakeShared<SendBuffer>(sizeof(CPacketHeader) + chat.ByteSizeLong());
	CPacketHandler::SerializeC_Chat(chat, sendbuf->GetBuffer());
	EnqueueSendPacket(sendbuf);
}
ClientSession::ClientSession(FSocket* pSocket)
	:m_pSocket(pSocket), m_pSendWorker(nullptr), m_pRecvWorker(nullptr)
{

}

ClientSession::~ClientSession()
{
	m_pSendWorker = nullptr;
	m_pRecvWorker = nullptr;
}
