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
class CPacketHandler;
class UCastleDefenseGameInstance;
class CASTLEDEFENSE_API ClientSession : public TSharedFromThis<ClientSession>
{
public:
	void EnqueueRecvPacket(TArray<uint8>);
	void DequeueRecvPacket();

	void EnqueueSendPacket(TSharedPtr<SendBuffer>);
	bool DequeueSendPacket(TSharedPtr<SendBuffer>&);

	void CreateWorkers();

	void SendC_Login();
	UFUNCTION(BlueprintCallable)
	void SendC_Spawn();
	void SendC_Despawn();
	void SendC_Move(Coordiante*, Rotation*, MoveState, bool);
	void SendC_Chat(char* pBuf);
	void SendC_Attack(int);
	ClientSession(FSocket*, UCastleDefenseGameInstance*);
	~ClientSession();

public:
	Player m_player;
	TSharedPtr<CPacketHandler> m_pPacketHandler;

	TSharedPtr<RecvWorker> m_pRecvWorker;
	TSharedPtr<SendWorker> m_pSendWorker;
	FSocket* m_pSocket;
	TQueue<TArray<uint8>> m_recvQueue;
	TQueue<TSharedPtr<SendBuffer>> m_sendQueue;
};
