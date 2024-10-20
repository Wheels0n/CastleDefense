// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

/**
 * 
 */
class UAISenseConfig_Sight;

UCLASS()
class CASTLEDEFENSE_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()
public :
	AEnemyAIController();

	UFUNCTION()
	void SetGotHit(bool bGotHit);
	UFUNCTION()
	void SetNewDest(bool bNewDest);

	UFUNCTION()
	void StopBehaviorTree();

	virtual void OnPossess(APawn* pPawn) override;
private:
	UFUNCTION()
	void OnTargetInSight(AActor* Actor, FAIStimulus const Stimulus);
	UFUNCTION()
	void StartEnemyTimer();
private:

	UPROPERTY(VisibleDefaultsOnly, Category = AI)
	UBlackboardData* m_pBB;

	UPROPERTY(VisibleDefaultsOnly, Category = AI)
	UBehaviorTree* m_pBT;

	UPROPERTY(VisibleDefaultsOnly, Category = AI)
	UAISenseConfig_Sight* m_pSightConfig;

	FTimerHandle m_hTimer;

};
