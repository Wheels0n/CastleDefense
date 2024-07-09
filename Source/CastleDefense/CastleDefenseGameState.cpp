// Fill out your copyright notice in the Description page of Project Settings.

#include "CastleDefenseGameState.h"
#include "SkeletonEnemy.h"
#include "Math/UnrealMathUtility.h"


ACastleDefenseGameState::ACastleDefenseGameState()
{
	for (int i = 0; i < 5; ++i)
	{
		m_enemies.Add(nullptr);
		m_enemyIdx.Add(nullptr, i);
	}
}

void ACastleDefenseGameState::SetDelete(ASkeletonEnemy* pEnemy)
{
	int* pIdx = m_enemyIdx.Find(pEnemy);
	if (pIdx != nullptr)
	{
		m_enemies[*pIdx] = nullptr;
		m_enemyIdx.Remove(pEnemy);
	}
}

void ACastleDefenseGameState::BeginPlay()
{
	UWorld* pWolrd = GetWorld();
	FTimerManager& timerManager = pWolrd->GetTimerManager();
	timerManager.SetTimer(m_hTimer, this, &ACastleDefenseGameState::CheckEnemyAlive, 10.0f,true);

}

void ACastleDefenseGameState::CheckEnemyAlive()
{
	FMath fmath;
	for(int i=0;i<5;++i)
	{
		if (m_enemies[i] == nullptr)
		{
			UWorld* pWolrd = GetWorld();
			float x = fmath.RandRange(-100.0f, 100.0f);
			float y = fmath.RandRange(-100.0f, 100.0f);
			FVector location = FVector(x, y, 88.0f);
			FActorSpawnParameters spawnParams;
			spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			m_enemies[i] = pWolrd->SpawnActor<ASkeletonEnemy>(spawnParams);
			check(m_enemies[i] != nullptr);
			m_enemies[i]->SetActorLocation(location);

			m_enemyIdx.Add(m_enemies[i], i);
		}
	}
}

