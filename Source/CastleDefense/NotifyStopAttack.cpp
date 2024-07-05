// Fill out your copyright notice in the Description page of Project Settings.


#include "NotifyStopAttack.h"
#include "SkeletonEnemy.h"

void UNotifyStopAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	GEngine->AddOnScreenDebugMessage(-3, 1.0f, FColor::Green, TEXT("Notified"));
	UObject* pOuter = MeshComp->GetOuter();
	if (pOuter->IsA(ASkeletonEnemy::StaticClass()))
	{
		GEngine->AddOnScreenDebugMessage(-4, 1.0f, FColor::Green, TEXT("StopAttack"));
		ASkeletonEnemy* pEnemy = Cast<ASkeletonEnemy>(pOuter);
		pEnemy->StopAttack();
	}
	

}
