// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Network/test.pb.h"
/**
 * 
 */
class FSocket;
class RecvWorker;
class SendWorker;
class SendBuffer;
class CASTLEDEFENSE_API ClientSession : public TSharedFromThis<ClientSession>
{
public:
	void EnqueueRecvPacket(TArray<uint8>);
	void DequeueRecvPacket();

	void EnqueueSendPacket(TSharedPtr<SendBuffer>);
	bool DequeueSendPacket(TSharedPtr<SendBuffer>&);

	void CreateWorkers();
	void SendC_Chat(char* pBuf);
	ClientSession(FSocket*);
	~ClientSession();

public:
	TSharedPtr<RecvWorker> m_pRecvWorker;
	TSharedPtr<SendWorker> m_pSendWorker;
	FSocket* m_pSocket;
	TQueue<TArray<uint8>> m_recvQueue;
	TQueue<TSharedPtr<SendBuffer>> m_sendQueue;
};
