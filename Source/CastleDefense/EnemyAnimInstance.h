// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyAnimInstance.generated.h"

/**
 * 
 */

class APawn;
class UAnimMontage;

UCLASS()
class CASTLEDEFENSE_API UEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UEnemyAnimInstance();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTimeX) override;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	UAnimMontage* m_pHitMontage;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float m_speed;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float m_dir;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool m_bAttacking;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool m_bDead;
private:
	APawn* m_pOwner;
};
