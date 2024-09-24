// Fill out your copyright notice in the Description page of Project Settings.

#include "CastleDefenseGameState.h"
#include "SkeletonEnemy.h"
#include "Wizard.h"
#include "Network/test.pb.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"


ACastleDefenseGameState::ACastleDefenseGameState()
{
	for (int i = 0; i < 5; ++i)
	{
		//.Add(nullptr);
		//m_enemyIdx.Add(nullptr, i);
	}
}

void ACastleDefenseGameState::SetDeleteEnemy(ASkeletonEnemy* pEnemy)
{
	int* pIdx = m_enemyIdx.Find(pEnemy);
	if (pIdx != nullptr)
	{
		m_enemies[*pIdx] = nullptr;
		m_enemyIdx.Remove(pEnemy);
	}
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

void ACastleDefenseGameState::UpdatePlayerPos(Player* pPlayer)
{
	AWizard* pCurPlayer = m_idToPlayer[pPlayer->id()];
	pCurPlayer->SetNewDest(pPlayer);
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

