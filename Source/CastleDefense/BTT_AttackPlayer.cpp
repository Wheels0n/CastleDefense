// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_AttackPlayer.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "SkeletonEnemy.h"
UBTT_AttackPlayer::UBTT_AttackPlayer()
{
}

EBTNodeResult::Type UBTT_AttackPlayer::ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* pNodeMemory)
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

	ASkeletonEnemy* pEnemy = Cast<ASkeletonEnemy>(pCharacter);
	if (!pEnemy->IsAttacking())
	{
		pEnemy->StartAttack();
	}
	
	return EBTNodeResult::Succeeded;
}

