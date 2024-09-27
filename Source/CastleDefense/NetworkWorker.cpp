// Fill out your copyright notice in the Description page of Project Settings.


#include "NetworkWorker.h"
#include "ClientSession.h"
#include "HAL/Runnable.h"
#include "Sockets.h"
#include "SendBuffer.h"
#include "CPacketHandler.h"
bool RecvWorker::Init()
{
	return true;
}

uint32 RecvWorker::Run()
{
	while (m_bRunning)
	{
		RecvPacket();
	}
	return 0;
}

void RecvWorker::Exit()
{

}

RecvWorker::RecvWorker(FSocket* pSocket, TSharedPtr<ClientSession> session)
	:m_pThread(nullptr), m_bRunning(true), m_pSocket(pSocket), m_session(session)
{
	m_pThread = FRunnableThread::Create(this, TEXT("RecvWorker"));
}

RecvWorker::~RecvWorker()
{
	m_bRunning = false;
	m_pSocket = nullptr;
	m_pThread->Kill();
}

void RecvWorker::RecvPacket()
{
	uint32 pendingBytes = 0;
	if (!m_pSocket->HasPendingData(pendingBytes)||pendingBytes==0)
	{
		return;
	}

	int headerSize = sizeof(CPacketHeader);
	TArray<uint8> buffer;
	buffer.AddZeroed(headerSize);
	int recvBytes = 0;
	uint8* pos = &buffer[0];

	while (headerSize)
	{
		bool bSucceded = 
			m_pSocket->Recv(pos, headerSize, recvBytes);
		if (!bSucceded)
		{
			UE_LOG(LogTemp, Error, TEXT("RecvPacket() Failed"));
			return;
		}
		UE_LOG(LogTemp, Error, TEXT("RecvBytes : %d"), recvBytes);
		pos += recvBytes;
		headerSize -= recvBytes;
		recvBytes = 0;
	}
	
	CPacketHeader* pHeader = reinterpret_cast<CPacketHeader*>(&buffer[0]);
	UE_LOG(LogTemp, Error, TEXT("PacketSize : %d"), pHeader->size);
	UE_LOG(LogTemp, Error, TEXT("PacketType : %d"), pHeader->id);
	headerSize = sizeof(CPacketHeader);
	int payloadSize = pHeader->size - headerSize;
	if (payloadSize)
	{
		buffer.AddZeroed(payloadSize);

		pos = &buffer[headerSize];
		while (payloadSize)
		{
			bool bSucceded =
				m_pSocket->Recv(pos, payloadSize, recvBytes);
			if (!bSucceded)
			{
				UE_LOG(LogTemp, Error, TEXT("RecvPacket() Failed"));
				return;
			}
			UE_LOG(LogTemp, Error, TEXT("RecvBytes : %d"), recvBytes);
			pos += recvBytes;
			payloadSize -= recvBytes;
			recvBytes = 0;
		}
	}
	
	UE_LOG(LogTemp, Error, TEXT("RecvPacket() Succeded"));
	TSharedPtr<ClientSession> pSession = m_session.Pin();
	pSession->EnqueueRecvPacket(buffer);
}

bool SendWorker::Init()
{
	return true;
}

uint32 SendWorker::Run()
{
	while (m_bRunning)
	{
		
		if (m_session != nullptr)
		{
			TSharedPtr<ClientSession> pSession = m_session.Pin();
			TSharedPtr<SendBuffer> pkt;
			if (pSession->DequeueSendPacket(pkt))
			{
				UE_LOG(LogTemp, Display, TEXT("SendQueue Dequeue"));
				SendPacket(pkt);
			}
		}
		
		
	}
	return 0;
}

void SendWorker::Exit()
{
	
}

SendWorker::SendWorker(FSocket* pSocket, TSharedPtr<ClientSession> session)
	:m_pThread(nullptr), m_bRunning(true), m_pSocket(pSocket), m_session(session)
{
	m_pThread = FRunnableThread::Create(this, TEXT("SendWorker"));
}

SendWorker::~SendWorker()
{
	m_session = nullptr;
	m_pSocket = nullptr;
	m_bRunning = false;
	m_pThread->Kill();
}

void SendWorker::SendPacket(TSharedPtr<SendBuffer> pkt)
{

	uint8* pos = (uint8*)pkt->GetBuffer();
	int toSend = pkt->GetSize();
	int sentBytes = 0;
	while (toSend)
	{
		bool bSucceded =
			m_pSocket->Send(pos, toSend, sentBytes);
		if (!bSucceded)
		{
			UE_LOG(LogTemp, Error, TEXT("SendPacket() Failed"));
			return;
		}
		pos += sentBytes;
		toSend -= sentBytes;
		sentBytes = 0;
	}
	UE_LOG(LogTemp, Log, TEXT("SendPacket() Succeded"));
	CPacketHeader* pHeader = reinterpret_cast<CPacketHeader*>(pkt->GetBuffer());
	if (pHeader->id == Despawn)
	{
		FGenericPlatformMisc::RequestExit(false);
	}

}
