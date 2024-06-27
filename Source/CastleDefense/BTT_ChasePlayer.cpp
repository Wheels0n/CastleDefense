// Fill out your copyright notice in the Description page of Project Settings.

#include "BTT_ChasePlayer.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
UBTT_ChasePlayer::UBTT_ChasePlayer()
	:m_chaseSpeed(500.0f)
{
}

EBTNodeResult::Type UBTT_ChasePlayer::ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* pNodeMemory)
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

	UCharacterMovementComponent* pMoveComp= pCharacter->GetCharacterMovement();
	if (pMoveComp == nullptr)
	{
		return EBTNodeResult::Failed;
	}
	
	pMoveComp->MaxWalkSpeed = m_chaseSpeed;


	return EBTNodeResult::Succeeded;
}
