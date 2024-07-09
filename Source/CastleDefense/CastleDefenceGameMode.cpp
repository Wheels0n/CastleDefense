// Fill out your copyright notice in the Description page of Project Settings.


#include "CastleDefenceGameMode.h"
#include "Wizard.h"

ACastleDefenceGameMode::ACastleDefenceGameMode()
{
	static ConstructorHelpers::FClassFinder<AWizard> wizardClass(TEXT("/Script/CoreUObject.Class'/Script/CastleDefense.Wizard'"));
	DefaultPawnClass = wizardClass.Class;
}

void ACastleDefenceGameMode::BeginPlay()
{
	Super::BeginPlay();
	OnPlayerDied.AddDynamic(this, &ACastleDefenceGameMode::PlayerDied);
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
