// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAnimInstance.h"

UEnemyAnimInstance::UEnemyAnimInstance()
	:m_speed(0.0f), m_dir(0.0f)
{
}

void UEnemyAnimInstance::NativeInitializeAnimation()
{
	UAnimInstance::NativeInitializeAnimation();
	m_pOwner = TryGetPawnOwner();
}

void UEnemyAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
	UAnimInstance::NativeUpdateAnimation(DeltaTimeX);

	if (m_pOwner && m_pOwner->IsA(ACharacter::StaticClass()))
	{
		ACharacter* pCharacter = Cast<ACharacter>(m_pOwner);
		if (pCharacter)
		{
			FVector  velocity = pCharacter->GetVelocity();
			FRotator rotation = pCharacter->GetActorRotation();

			m_dir = CalculateDirection(velocity, rotation);
			m_speed = velocity.Size();
		}
	}
}
