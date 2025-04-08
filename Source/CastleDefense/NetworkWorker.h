// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
struct CPacketHeader
{
	//크기는 헤더+페이로드
	int size;
	int id;
};

class SendBuffer;
class FSocket;
class ClientSession;
class CASTLEDEFENSE_API RecvWorker : public FRunnable
{
public:
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Exit() override;

	void Shutdown() { m_bRunning = false; m_pThread->WaitForCompletion(); };
	RecvWorker(FSocket*, TWeakPtr<ClientSession> session);
	~RecvWorker();
private:
	void RecvPacket();
protected:
	TWeakPtr<ClientSession> m_session;
	FSocket* m_pSocket;
	FRunnableThread* m_pThread;
	bool m_bRunning;
};

class CASTLEDEFENSE_API SendWorker : public FRunnable
{
public:
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Exit() override;

	void Shutdown() { m_bRunning = false; m_pThread->WaitForCompletion();};
	SendWorker(FSocket*, TWeakPtr<ClientSession> session);
	~SendWorker();
private:
	void SendPacket(TSharedPtr<SendBuffer>);

protected:
	TWeakPtr<ClientSession> m_session;
	FSocket*			m_pSocket;
	FRunnableThread*	m_pThread;
	bool m_bRunning;
};