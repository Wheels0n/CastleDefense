// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAnimInstance.h"
#include "SkeletonEnemy.h"
UEnemyAnimInstance::UEnemyAnimInstance()
	:m_speed(0.0f), m_dir(0.0f), m_bAttacking(false)
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

	if (m_pOwner && m_pOwner->IsA(ASkeletonEnemy::StaticClass()))
	{
		ASkeletonEnemy* pEnemy = Cast<ASkeletonEnemy>(m_pOwner);
		if (pEnemy)
		{
			FVector  velocity = pEnemy->GetVelocity();
			FRotator rotation = pEnemy->GetActorRotation();

			m_dir = CalculateDirection(velocity, rotation);
			m_speed = velocity.Size();
			m_bAttacking = pEnemy->IsAttacking();
		}
	}
}
