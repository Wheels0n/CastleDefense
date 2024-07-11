// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WizardAnimInstance.generated.h"

class APawn;
class UAnimMontage;
/**
 * 
 */
UCLASS()
class CASTLEDEFENSE_API UWizardAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public: 
	UWizardAnimInstance();


	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	UAnimMontage* m_pAttackMontage;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	UAnimMontage* m_pHitMontage;

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTimeX) override;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float m_speed;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float m_dir;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool m_bInAir;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool m_bDead;

private:

private:
	APawn* m_pOwner;
};
