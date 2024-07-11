// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Wizard.generated.h"

class UWizardWidget;
class UCameraComponent;

UCLASS()
class CASTLEDEFENSE_API AWizard : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AWizard();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

public:	
	bool IsAttacking() { return m_bAttacking; };
	bool IsDead() { return m_bDead; };
	bool IsDestroying() { return m_bDestroySet; };
	bool IsHit() { return m_bGotHit; };

	void CheckPlayerAttack();
	void DecreaseHp();
	void ResetHit() { m_bGotHit = false; };
	void DestroyTimer();
	void DestroyPlayer();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
private:
	UFUNCTION()
	void MoveForward(float value);

	UFUNCTION()
	void MoveRight(float value);

	UFUNCTION()
	void StartAttack();

	UFUNCTION()
	void StopAttack();

	UFUNCTION()
	void StartSprint();

	UFUNCTION()
	void StopSprint();

	UFUNCTION()
	void StartJump();

	UFUNCTION()
	void StopJump();

private: 
	UPROPERTY(VisibleDefaultsOnly, Category = Widget)
	TSubclassOf<UWizardWidget> WidgetClass;

	UPROPERTY(VisibleDefaultsOnly, Category = Widget)
	UWizardWidget* m_pWidget;

	UPROPERTY(VisibleDefaultsOnly, Category = Item)
	AActor* m_pWeapon;

	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* m_pSkeletalMeshComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = Camera)
	UCameraComponent* m_pCamComponent;

	FTimerHandle m_hTimer;

	bool m_bAttacking;
	bool m_bGotHit;
	bool m_bDead;
	bool m_bDestroySet;
	int m_Hp;
};
