// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "SkeletonEnemy.generated.h"

UCLASS()
class CASTLEDEFENSE_API ASkeletonEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASkeletonEnemy();
	UFUNCTION(BlueprintCallable)
	void StartAttack();
	
	UFUNCTION(BlueprintCallable)
	void StopAttack();
	
	UFUNCTION(BlueprintCallable)
	bool IsAttacking() { return m_bAttacking; };

	UFUNCTION(BlueprintCallable)
	void SetHit() { m_bHit = true; };

	UFUNCTION(BlueprintCallable)
	bool IsHitAlready() { return m_bHit; };
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UFUNCTION()
	void OnHandOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
private:
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* m_pSkeletalMeshComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = Collision)
	USphereComponent* m_pSphereComponent;

	bool m_bAttacking;
	bool m_bHit;
	int m_Hp;

};
