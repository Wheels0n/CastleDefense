// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CastleDefenseGameInstance.generated.h"

class ClientSession;
class FSocket;
UCLASS()
class CASTLEDEFENSE_API UCastleDefenseGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	virtual void Init() override;
	virtual void Shutdown() override;

	TSharedPtr<ClientSession> GetSession() { return m_pSession; };

	int32 GetUserID();

	UFUNCTION(BlueprintCallable)
	bool ConnectToServer();

	UFUNCTION(BlueprintCallable)
	void DisconnectFromServer();

	void SendMessage(char* pBuf);

	UFUNCTION(BlueprintCallable)
	void SpawnPlayer();

	UFUNCTION(BlueprintCallable)
	void DequeuePacket();

	UCastleDefenseGameInstance();
private:
	

private:
	FSocket* m_pSocket;
	TSharedPtr<ClientSession> m_pSession;
	FString m_ipAddress;
	int m_port;
};
