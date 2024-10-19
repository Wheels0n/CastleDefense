// Fill out your copyright notice in the Description page of Project Settings.

#include "CastleDefenseGameState.h"
#include "SkeletonEnemy.h"
#include "Wizard.h"
#include "Network/test.pb.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"

const int32 NUM_ENEMY = 5;
ACastleDefenseGameState::ACastleDefenseGameState()
{

}

void ACastleDefenseGameState::AddPlayer(Player* pPlayerInfo, int id)
{
	UWorld* pCurWolrd = GetWorld();
	Coordiante coord = pPlayerInfo->coord();
	FVector location = FVector(coord.x(), coord.y(), coord.z());
	FRotator rotation = FRotator::ZeroRotator;
	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
	AWizard* pPlayer = pCurWolrd->SpawnActor<AWizard>(location, rotation, spawnParams);

	m_idToPlayer.Add(pPlayerInfo->id(), pPlayer);
	
	if (id == pPlayerInfo->id())
	{
		APlayerController* pController = UGameplayStatics::GetPlayerController(pCurWolrd, 0);
		if (pController)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 23.0f, FColor::Yellow, FString::Printf(TEXT("Possessed : %d"), pPlayerInfo->id()));
			pController->UnPossess();
			pController->Possess(pPlayer);
			pPlayer->LazyCreateWidget();
		}
	}
	
}

void ACastleDefenseGameState::AddEnemy(Enemy* pEnemyInfo)
{
	int32 idx = pEnemyInfo->id();
	if (m_enemies.Num() == 0)
	{
		m_enemies.AddZeroed(NUM_ENEMY);
	}
	Coordiante coord = pEnemyInfo->coord();
	FVector location = FVector(coord.x(), coord.y(), coord.z());
	FRotator rotation = FRotator::ZeroRotator;

	UWorld* pCurWorld = GetWorld();
	m_enemies[idx] = pCurWorld->SpawnActor<ASkeletonEnemy>(location,rotation);
	m_enemyPtrToIdx.Add(m_enemies[idx],idx);
	m_enemyPtrToIdx[m_enemies[idx]] = idx;
}

int ACastleDefenseGameState::GetEnemyIndexByPtr(ASkeletonEnemy* pEnemy)
{
	return m_enemyPtrToIdx.Find(pEnemy) == nullptr ?
		-1 : m_enemyPtrToIdx[pEnemy];
}

void ACastleDefenseGameState::UpdateEnemyHp(int idx)
{
	m_enemies[idx]->DecreaseHp();
}

void ACastleDefenseGameState::UpdateEnemyPos(Enemy* pEnemy, int idx)
{

	if (m_enemies.Num() == 0)
	{
		return;
	}
	ASkeletonEnemy* pCurEnemy = m_enemies[idx];
	pCurEnemy->SetDest(pEnemy);
}


AWizard* ACastleDefenseGameState::GetPlayerById(int id)
{
	return m_idToPlayer.Find(id)==nullptr?
		nullptr:m_idToPlayer[id];
}

void ACastleDefenseGameState::RemovePlayerById(int id)
{
	AWizard* pCurPlayer = m_idToPlayer[id];
	m_players.Remove(pCurPlayer);
	m_idToPlayer.Remove(id);
	if (pCurPlayer->Destroy())
	{
		UE_LOG(LogTemp, Log, TEXT("Player Quit"));
	}
}

void ACastleDefenseGameState::UpdatePlayerMovement(Player* pPlayer)
{
	AWizard* pCurPlayer = m_idToPlayer[pPlayer->id()];
	pCurPlayer->SetNewDest(pPlayer);
	
	if (pPlayer->battack())
	{
		pCurPlayer->StartAttack();
	}
	else
	{
		pCurPlayer->StopAttack();
	}
}

void ACastleDefenseGameState::BeginPlay()
{
	//UWorld* pWolrd = GetWorld();
	//FTimerManager& timerManager = pWolrd->GetTimerManager();
	//timerManager.SetTimer(m_hTimer, this, &ACastleDefenseGameState::CheckEnemyAlive, 10.0f,true);

}

void ACastleDefenseGameState::CheckEnemyAlive()
{
	FMath fmath;
	for(int i=0;i< NUM_ENEMY;++i)
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

			m_enemyPtrToIdx.Add(m_enemies[i], i);
		}
	}
}

