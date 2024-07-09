// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "CastleDefenseGameState.generated.h"

/**
 * 
 */
class ASkeletonEnemy;
UCLASS()
class CASTLEDEFENSE_API ACastleDefenseGameState : public AGameState
{
	GENERATED_BODY()
public:
	ACastleDefenseGameState();

	void SetDelete(ASkeletonEnemy*);
protected:
	virtual void BeginPlay() override;
private:
	void CheckEnemyAlive();
private:

	FTimerHandle m_hTimer;
	TArray<ASkeletonEnemy*> m_enemies;
	TMap<ASkeletonEnemy*,int> m_enemyIdx;
};
