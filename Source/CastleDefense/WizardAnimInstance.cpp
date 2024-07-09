// Fill out your copyright notice in the Description page of Project Settings.
#include "WizardAnimInstance.h"
#include "Wizard.h"
UWizardAnimInstance::UWizardAnimInstance()
	:m_speed(0.0f), m_dir(0.0f), m_bInAir(false), m_bDead(false)
{
	static ConstructorHelpers::FObjectFinder<UAnimMontage>
		attackMontageAsset(TEXT("/Script/Engine.AnimMontage'/Game/BattleWizardPolyart/Animations/WizardAttackMontage.WizardAttackMontage'"));
	if (attackMontageAsset.Succeeded())
	{
		m_pAttackMontage = attackMontageAsset.Object;
	}
	check(m_pAttackMontage != nullptr);

	static ConstructorHelpers::FObjectFinder<UAnimMontage>
		hitMontageAsset(TEXT("/Script/Engine.AnimMontage'/Game/BattleWizardPolyart/Animations/WizardHitMontage.WizardHitMontage'"));
	if (hitMontageAsset.Succeeded())
	{
		m_pHitMontage = hitMontageAsset.Object;
	}
	check(m_pHitMontage != nullptr);

}

void UWizardAnimInstance::NativeInitializeAnimation()
{
	UAnimInstance::NativeInitializeAnimation();
	m_pOwner = TryGetPawnOwner();
}

void UWizardAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
	UAnimInstance::NativeUpdateAnimation(DeltaTimeX);

	if (m_pOwner&&m_pOwner->IsA(ACharacter::StaticClass()))
	{
		AWizard* pCharacter = Cast<AWizard>(m_pOwner);
		if (pCharacter)
		{
			m_bInAir = pCharacter->GetMovementComponent()->IsFalling();
			
			FVector  velocity = pCharacter->GetVelocity();
			FRotator rotation = pCharacter->GetActorRotation();
			
			m_dir = CalculateDirection(velocity, rotation);
			m_speed = velocity.Size();
			
			if (pCharacter->IsDead())
			{
				if (!pCharacter->IsDestroying())
				{
					GEngine->AddOnScreenDebugMessage(-11, 1.0f, FColor::Red, TEXT("PlayerDead"));
					m_bDead = true;
				}
				return;
			}

			if (pCharacter->IsHit() && !Montage_IsPlaying(m_pHitMontage))
			{
				GEngine->AddOnScreenDebugMessage(-8, 1.0f, FColor::Yellow, TEXT("PlayHitMontage"));
				Montage_Play(m_pHitMontage);
				
			}
			else if (pCharacter->IsAttacking() && !Montage_IsPlaying(m_pAttackMontage))
			{
				Montage_Play(m_pAttackMontage);
			}
		}
	}
}
