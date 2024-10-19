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
class Enemy;

UCLASS()
class CASTLEDEFENSE_API ACastleDefenseGameState : public AGameState
{
	GENERATED_BODY()
public:
	ACastleDefenseGameState();

	void AddEnemy(Enemy*);
	int GetEnemyIndexByPtr(ASkeletonEnemy*);
	void UpdateEnemyHp(int);
	void UpdateEnemyPos(Enemy*, int);

	AWizard* GetPlayerById(int);
	void AddPlayer(Player*, int);
	void RemovePlayerById(int);
	void UpdatePlayerMovement(Player*);
protected:
	virtual void BeginPlay() override;
private:
	void CheckEnemyAlive();
private:

	FTimerHandle m_hTimer;
	TArray<ASkeletonEnemy*> m_enemies;
	TArray<AWizard*> m_players;
	TMap<ASkeletonEnemy*,int> m_enemyPtrToIdx;
	TMap<int, AWizard*>m_idToPlayer;
};
