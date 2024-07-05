// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_AttackPlayer.generated.h"

/**
 * 
 */
UCLASS()
class CASTLEDEFENSE_API UBTT_AttackPlayer : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTT_AttackPlayer();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* pNodeMemory);
private:
	FTimerHandle m_hTimer;
};
