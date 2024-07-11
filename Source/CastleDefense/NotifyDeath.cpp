// Fill out your copyright notice in the Description page of Project Settings.

#include "NotifyDeath.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Wizard.h"
#include "SkeletonEnemy.h"
void UNotifyDeath::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	UObject* pOuter = MeshComp->GetOuter();
	if (pOuter->IsA(AWizard::StaticClass()))
	{
		UE_LOG(LogTemp, Display, TEXT("NotifyDeath"));
		AWizard* pCharacter = Cast<AWizard>(pOuter);
		if (pCharacter)
		{
			pCharacter->DestroyTimer();
		}
	}
	else if (pOuter->IsA(ASkeletonEnemy::StaticClass()))
	{
		ASkeletonEnemy* pCharacter = Cast<ASkeletonEnemy>(pOuter);
		if (pCharacter)
		{
			pCharacter->DestroyTimer();
		}
	}
}
