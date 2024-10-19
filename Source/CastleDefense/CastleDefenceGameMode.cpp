// Fill out your copyright notice in the Description page of Project Settings.


#include "CastleDefenceGameMode.h"
#include "Wizard.h"
#include "Runtime/NavigationSystem/Public/NavigationSystem.h"
#include "Runtime/NavigationSystem/Public/NavMesh/RecastNavMesh.h"
#include "Runtime/NavigationSystem/Public/NavMesh/RecastNavMeshGenerator.h"
ACastleDefenceGameMode::ACastleDefenceGameMode()
{
}

void ACastleDefenceGameMode::BeginPlay()
{
	Super::BeginPlay();
	OnPlayerDied.AddDynamic(this, &ACastleDefenceGameMode::PlayerDied);

	/*auto NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	ANavigationData* pNavData =
		NavSys->GetDefaultNavDataInstance(FNavigationSystem::ECreateIfEmpty::DontCreate);

	ARecastNavMesh* pNavMesh = Cast<ARecastNavMesh>(pNavData);
	pNavMesh->GetGenerator()->ExportNavigationData(FString(TEXT("TestNavMesh")));*/
}

void ACastleDefenceGameMode::RestartPlayer(AController* pController)
{
	Super::RestartPlayer(pController);
}

void ACastleDefenceGameMode::PlayerDied(AController* pController)
{
	UE_LOG(LogTemp, Display, TEXT("Respawn"));
	check(pController != nullptr);
	RestartPlayer(pController);
}
