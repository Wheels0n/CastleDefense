// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BehaviorTree.h"
AEnemyAIController::AEnemyAIController()
{
	static ConstructorHelpers::FObjectFinder<UBlackboardData> 
		enemyBBAsset(TEXT("/Script/AIModule.BlackboardData'/Game/Enemy_BB.Enemy_BB'"));
	if (enemyBBAsset.Succeeded())
	{
		m_pBB = enemyBBAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UBehaviorTree>
		enemyBTAsset(TEXT("/Script/AIModule.BehaviorTree'/Game/Enemy_BT.Enemy_BT'"));
	if (enemyBTAsset.Succeeded())
	{
		m_pBT = enemyBTAsset.Object;
	}

	Blackboard = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardData"));
	check(Blackboard != nullptr);

	//Set AIPerception
	m_pSightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("AISightConfig"));
	check(m_pSightConfig != nullptr);

	m_pSightConfig->SightRadius = 3000.0f;
	m_pSightConfig->LoseSightRadius = 3500.0f;
	m_pSightConfig->PeripheralVisionAngleDegrees = 90.0f;

	m_pSightConfig->DetectionByAffiliation.bDetectEnemies = true;
	m_pSightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	m_pSightConfig->DetectionByAffiliation.bDetectFriendlies = false;

	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPeceptionComp"));
	check(PerceptionComponent!=nullptr);
	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnTargetInSight);
	PerceptionComponent->ConfigureSense(*m_pSightConfig);
}


void AEnemyAIController::OnPossess(APawn* pPawn)
{
	Super::OnPossess(pPawn);
	if (m_pBB)
	{
		Blackboard->InitializeBlackboard(*m_pBB);
		if (m_pBT)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("OnPossed"));
			RunBehaviorTree(m_pBT);
		}
		
	}
}

void AEnemyAIController::OnTargetInSight(AActor* pActor, FAIStimulus const Stimulus)
{
	bool bWizard = pActor->ActorHasTag(TEXT("Wizard"));
	bool bSensed = Stimulus.WasSuccessfullySensed();

	if (bWizard && bSensed)
	{
		GEngine->AddOnScreenDebugMessage(0, 1.0f, FColor::Green, TEXT("TargetFound"));
		m_hTimer.Invalidate();
		Blackboard->SetValueAsBool(FName(TEXT("bInSight")), true);
		Blackboard->SetValueAsObject(FName(TEXT("Wizard")), pActor);
	}
	else
	{
		
		Blackboard->SetValueAsBool(FName(TEXT("bInMeleeRange")), false);
		GEngine->AddOnScreenDebugMessage(1, 1.0f, FColor::Green, TEXT("TargetMissing"));
		UWorld* pWolrd = pActor->GetWorld();
		FTimerManager& timerManager = pWolrd->GetTimerManager();
		timerManager.SetTimer(m_hTimer, this, &AEnemyAIController::StartEnemyTimer, 4.0f);

	}
}

void AEnemyAIController::SetGotHit(bool bGotHit)
{
	Blackboard->SetValueAsBool(FName(TEXT("bGotHit")), bGotHit);
}

void AEnemyAIController::StartEnemyTimer()
{
	GEngine->AddOnScreenDebugMessage(2,1.0f, FColor::Green, TEXT("SetTimer"));
	Blackboard->SetValueAsBool(FName(TEXT("bInSight")), false);
	Blackboard->SetValueAsObject(FName(TEXT("Wizard")), nullptr);
}
