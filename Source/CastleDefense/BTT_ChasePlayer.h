// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_ChasePlayer.generated.h"

/**
 * 
 */
UCLASS()
class CASTLEDEFENSE_API UBTT_ChasePlayer : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTT_ChasePlayer();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* pNodeMemory);
private:
	UPROPERTY(EditAnywhere)
	float m_chaseSpeed;
};
