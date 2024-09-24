// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "CastleDefenseGameState.generated.h"

/**
 * 
 */
class ASkeletonEnemy;
class AWizard;
class Player;
UCLASS()
class CASTLEDEFENSE_API ACastleDefenseGameState : public AGameState
{
	GENERATED_BODY()
public:
	ACastleDefenseGameState();

	void SetDeleteEnemy(ASkeletonEnemy*);
	void AddPlayer(Player*, int);
	AWizard* GetPlayerById(int);
	void RemovePlayerById(int);
	void UpdatePlayerPos(Player*);
protected:
	virtual void BeginPlay() override;
private:
	void CheckEnemyAlive();
private:

	FTimerHandle m_hTimer;
	TArray<ASkeletonEnemy*> m_enemies;
	TArray<AWizard*> m_players;
	TMap<ASkeletonEnemy*,int> m_enemyIdx;
	TMap<int, AWizard*>m_idToPlayer;
};
