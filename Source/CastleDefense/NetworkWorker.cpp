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
	{;
		//uint64 s = FPlatformTime::Cycles64();
		RecvPacket();
		//uint64 e = FPlatformTime::Cycles64();
		//UE_LOG(LogTemp, Log, TEXT("Recv Time : %f ms"), FPlatformTime::ToMilliseconds(e - s));
	}
	return 0;
}

void RecvWorker::Exit()
{

}

RecvWorker::RecvWorker(FSocket* pSocket, TWeakPtr<ClientSession> session)
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
	//TODO: 소멸자 호출전에 소켓이 해제 됨  
	TSharedPtr<ClientSession> pSession = m_session.Pin();
	if (pSession == nullptr)
	{
		return;
	}

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
		pos += recvBytes;
		headerSize -= recvBytes;
		recvBytes = 0;
	}
	
	CPacketHeader* pHeader = reinterpret_cast<CPacketHeader*>(&buffer[0]);

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
			pos += recvBytes;
			payloadSize -= recvBytes;
			recvBytes = 0;
		}
	}
	
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
		TSharedPtr<ClientSession> pSession = m_session.Pin();
		if (pSession != nullptr)
		{
			TSharedPtr<SendBuffer> pkt;

			if (pSession->DequeueSendPacket(pkt))
			{
				uint64 s = FPlatformTime::Cycles64();
				SendPacket(pkt);
				uint64 e = FPlatformTime::Cycles64();
				UE_LOG(LogTemp, Log, TEXT("Send Time : %f ms"), FPlatformTime::ToMilliseconds(e - s));
				
			}
		}
		
		
	}
	return 0;
}

void SendWorker::Exit()
{
	
}

SendWorker::SendWorker(FSocket* pSocket, TWeakPtr<ClientSession> session)
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
	CPacketHeader* pHeader = reinterpret_cast<CPacketHeader*>(pos);
	//모아 두었다 한꺼번에? 
	int toSend = pkt->GetSize();
	int sentBytes = 0;
	int i = 0;
	while (toSend)
	{
		uint64 s = FPlatformTime::Cycles64();
		bool bSucceded =
			m_pSocket->Send(pos, toSend, sentBytes);
		uint64 e = FPlatformTime::Cycles64();
		UE_LOG(LogTemp, Log, TEXT("Send Time : %f ms, %d th"), FPlatformTime::ToMilliseconds(e - s),i++);
		
		if (!bSucceded)
		{
			UE_LOG(LogTemp, Error, TEXT("SendPacket() Failed"));
			return;
		}
		pos += sentBytes;
		toSend -= sentBytes;
		sentBytes = 0;
	}

}
