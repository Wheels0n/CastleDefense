// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "CastleDefenceGameMode.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDiedSignature, AController*, Wizard);


UCLASS()
class CASTLEDEFENSE_API ACastleDefenceGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	ACastleDefenceGameMode();

protected:

};
