// Fill out your copyright notice in the Description page of Project Settings.

#include "CastleDefenseGameInstance.h"
#include "ClientSession.h"
#include "Networking.h"
#include "CPacketHandler.h"
#include "SendBuffer.h"
void UCastleDefenseGameInstance::Init()
{
	UGameInstance::Init();
	UE_LOG(LogInit, Display, TEXT("UCastleDefenseGameInstance::Init()"));
}

void UCastleDefenseGameInstance::Shutdown()
{
	UGameInstance::Shutdown();
	UE_LOG(LogTemp, Display, TEXT("UCastleDefenseGameInstance::Shutdown()"));
	if (m_pSocket != nullptr)
	{
		m_pSession = nullptr;

		ISocketSubsystem* pSocketSubsystem =
			ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
		pSocketSubsystem->DestroySocket(m_pSocket);
		m_pSocket = nullptr;
	}
}

UCastleDefenseGameInstance::UCastleDefenseGameInstance()
	:m_pSocket(nullptr), m_ipAddress(TEXT("127.0.0.1")), m_port(777), m_pSession(nullptr)
{
	
}

int32 UCastleDefenseGameInstance::GetUserID()
{
	return m_pSession==nullptr?
		0:m_pSocket->GetPortNo();
}

bool UCastleDefenseGameInstance::ConnectToServer()
{
	ISocketSubsystem* pSocketSubsystem =
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	m_pSocket = pSocketSubsystem->
		CreateSocket(TEXT("Stream"), TEXT("Client Socket"));

	FIPv4Address ip;
	FIPv4Address::Parse(m_ipAddress, ip);

	TSharedRef<FInternetAddr> internetAddr = pSocketSubsystem->CreateInternetAddr();
	internetAddr->SetIp(ip.Value);
	internetAddr->SetPort(m_port);

	UE_LOG(LogInit, Display, TEXT("Created Socket"));

	bool bConnected = m_pSocket->Connect(*internetAddr);
	m_pSession = MakeShared<ClientSession>(m_pSocket, this);
	m_pSession->CreateWorkers();
	if (bConnected)
	{
		GEngine->AddOnScreenDebugMessage(1, 3.0f, FColor::Yellow, "Connected");
		UE_LOG(LogInit, Display, TEXT("ConnectSucceded"));
		m_pSession->SendC_Login();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(1, 3.0f, FColor::Yellow, "NotConnected");
		UE_LOG(LogInit, Display, TEXT("FailedToConnect"));
	}
	return bConnected;
}

void UCastleDefenseGameInstance::DisconnectFromServer()
{
	if (m_pSession != nullptr)
	{
		m_pSession->SendC_Despawn();
	}
}

void UCastleDefenseGameInstance::SendMessage(char* pBuf)
{
	if (m_pSession != nullptr)
	{
		m_pSession->SendC_Chat(pBuf);
	}
}

void UCastleDefenseGameInstance::SpawnPlayer()
{
	if (m_pSession != nullptr)
	{
		m_pSession->SendC_Spawn();
	}
}

void UCastleDefenseGameInstance::DequeuePacket()
{
	if (m_pSession != nullptr)
	{
		m_pSession->DequeueRecvPacket();
	}
}
