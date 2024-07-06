// Fill out your copyright notice in the Description page of Project Settings.


#include "NotifyPlayerAttackHit.h"
#include "Wizard.h"

void UNotifyPlayerAttackHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	UObject* pOuter = MeshComp->GetOuter();
	if (pOuter->IsA(AWizard::StaticClass()))
	{
		AWizard* pCharacter = Cast<AWizard>(pOuter);
		pCharacter->CheckPlayerAttack();
	}

}
