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

	UCastleDefenseGameInstance();
	

	UFUNCTION(BlueprintCallable)
	void ConnectToServer();
	UFUNCTION(BlueprintCallable)
	void DequeuePacket();
private:
	

private:
	FSocket* m_pSocket;
	TSharedPtr<ClientSession> m_pSession;
	FString m_ipAddress;
	int m_port;
};
