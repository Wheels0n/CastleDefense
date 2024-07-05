// Fill out your copyright notice in the Description page of Project Settings.


#include "NotifyHitEnd.h"
#include "Wizard.h"

void UNotifyHitEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	GEngine->AddOnScreenDebugMessage(-9, 1.0f, FColor::Yellow, TEXT("UNotifyHitEnd::Notify"));
	UObject* pOuter = MeshComp->GetOuter();
	if (pOuter->IsA(AWizard::StaticClass()))
	{
		AWizard* pCharacter = Cast<AWizard>(pOuter);
		pCharacter->ResetHit();
	}


}