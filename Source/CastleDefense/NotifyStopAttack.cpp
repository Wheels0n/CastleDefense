// Fill out your copyright notice in the Description page of Project Settings.


#include "NotifyStopAttack.h"
#include "SkeletonEnemy.h"

void UNotifyStopAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UE_LOG(LogTemp, Display, TEXT("UNotifyStopAttack::Notify"));
	UObject* pOuter = MeshComp->GetOuter();
	if (pOuter->IsA(ASkeletonEnemy::StaticClass()))
	{
		UE_LOG(LogTemp, Display, TEXT("StopAttack"));
		ASkeletonEnemy* pEnemy = Cast<ASkeletonEnemy>(pOuter);
		if (pEnemy)
		{
		  pEnemy->StopAttack();
		}
	}
	

}
