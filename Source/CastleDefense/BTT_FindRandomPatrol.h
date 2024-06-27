// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_FindRandomPatrol.generated.h"

/**
 * 
 */
UCLASS()
class CASTLEDEFENSE_API UBTT_FindRandomPatrol : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTT_FindRandomPatrol();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* pNodeMemory) override;
private:

	UPROPERTY(EditAnywhere)
	float m_patrolSpeed;
	UPROPERTY(EditAnywhere)
	float m_patrolRadius;
};
