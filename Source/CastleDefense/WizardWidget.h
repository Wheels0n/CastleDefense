// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WizardWidget.generated.h"

/**
 * 
 */
UCLASS()
class CASTLEDEFENSE_API UWizardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetHp(int hp) { m_Hp = hp; };
public:
	UPROPERTY(BlueprintReadOnly, Category = "Status")
	int m_Hp;
};
