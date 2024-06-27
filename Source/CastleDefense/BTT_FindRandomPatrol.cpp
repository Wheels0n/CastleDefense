// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_FindRandomPatrol.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"

UBTT_FindRandomPatrol::UBTT_FindRandomPatrol()
	:m_patrolSpeed(100.0f), m_patrolRadius(1000.0f)
{
}

EBTNodeResult::Type UBTT_FindRandomPatrol::ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* pNodeMemory)
{
	AAIController* pAIController = ownerComp.GetAIOwner();
	if (pAIController == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	ACharacter* pCharacter = pAIController->GetCharacter();
	if (pCharacter == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	UCharacterMovementComponent* pMoveComp = pCharacter->GetCharacterMovement();
	if (pMoveComp == nullptr)
	{
		return EBTNodeResult::Failed;
	}
	pMoveComp->MaxWalkSpeed = m_patrolSpeed;

	UWorld* pWorld = pCharacter->GetWorld();
	if (pWorld == nullptr)
	{
		return EBTNodeResult::Failed;
	}
	UNavigationSystemV1* pNavSys = UNavigationSystemV1::GetNavigationSystem(pWorld);
	if (pNavSys == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	FVector location = pMoveComp->GetActorLocation();
	FNavLocation dest;
	
	bool bFound = pNavSys->GetRandomReachablePointInRadius(location, m_patrolRadius, dest);
	UBlackboardComponent* pBBComp = pAIController->GetBlackboardComponent();
	if (pBBComp == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	pBBComp->SetValueAsVector(FName(TEXT("PatrolLocation")), bFound?dest.Location:location);
	GEngine->AddOnScreenDebugMessage(3, 5.0f, FColor::Green, TEXT("SetNewPatrolLocation"));


	return EBTNodeResult::Succeeded;
}
