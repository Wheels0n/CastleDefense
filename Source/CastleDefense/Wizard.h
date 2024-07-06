// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Animation/Animinstance.h"
#include "Wizard.generated.h"

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

public:	
	bool IsAttacking() { return m_bAttacking; };
	bool IsHit() { return m_bGotHit; };

	void CheckPlayerAttack();
	void DecreaseHp();
	void ResetHit() { m_bGotHit = false; };

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

	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* m_pSkeletalMeshComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = Camera)
	UCameraComponent* m_pCamComponent;

	bool m_bAttacking;
	bool m_bGotHit;
	bool m_bDead;
	int m_Hp;
};
