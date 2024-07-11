// Fill out your copyright notice in the Description page of Project Settings.


#include "NotifyHitEnd.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Wizard.h"
#include "SkeletonEnemy.h"

void UNotifyHitEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	UE_LOG(LogTemp, Display, TEXT("UNotifyHitEnd::Notify"));
	UObject* pOuter = MeshComp->GetOuter();
	if (pOuter->IsA(AWizard::StaticClass()))
	{
		AWizard* pCharacter = Cast<AWizard>(pOuter);
		if (pCharacter)
		{
			pCharacter->ResetHit();
		}
	}
	else if (pOuter->IsA(ASkeletonEnemy::StaticClass()))
	{
		ASkeletonEnemy* pCharacter = Cast<ASkeletonEnemy>(pOuter);
		if (pCharacter)
		{
			pCharacter->ResetHit();
		}
	}


}