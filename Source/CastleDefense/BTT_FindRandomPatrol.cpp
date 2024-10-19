// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_FindRandomPatrol.h"
#include "EnemyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SkeletonEnemy.h"
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
	AEnemyAIController* pEnemyAIController = Cast<AEnemyAIController>(pAIController);
	if (pAIController == nullptr)
	{
		UE_LOG(LogTemp, Display, TEXT("AIController == nullptr"));
		return EBTNodeResult::Failed;
	}


	ASkeletonEnemy* pEnemy = Cast<ASkeletonEnemy> (pAIController->GetCharacter());
	if (pEnemy == nullptr)
	{
		UE_LOG(LogTemp, Display, TEXT("pEnemy == nullptr"));
		return EBTNodeResult::Failed;
	}

	UCharacterMovementComponent* pMoveComp = pEnemy->GetCharacterMovement();
	if (pMoveComp == nullptr)
	{
		UE_LOG(LogTemp, Display, TEXT("pMoveComp == nullptr"));
		return EBTNodeResult::Failed;
	}
	//pMoveComp->MaxWalkSpeed = m_patrolSpeed;

	Coordiante* pCoord = pEnemy->GetDest();
	FVector location(pCoord->x(), pCoord->y(), pCoord->z());
	
	UBlackboardComponent* pBBComp = pAIController->GetBlackboardComponent();
	if (pBBComp == nullptr)
	{
		UE_LOG(LogTemp, Display, TEXT("pBBComp == nullptr"));
		return EBTNodeResult::Failed;
	}

	pBBComp->SetValueAsVector(FName(TEXT("PatrolLocation")), location);
	UE_LOG(LogTemp, Display, TEXT("SetNewPatrolLocation"));
	//pEnemyAIController->SetNewDest(false);
	return EBTNodeResult::Succeeded;
}
