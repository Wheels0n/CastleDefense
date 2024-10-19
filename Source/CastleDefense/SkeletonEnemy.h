// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Network/test.pb.h"
#include "SkeletonEnemy.generated.h"

class UEnemyWidget;
class UWidgetComponent;
class USphereComponent;
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
	bool IsDestroying() { return m_bDestroySet; };
	bool IsDead() { return m_bDead; };
	bool IsHit() { return m_bGotHit; };
	void SetHit();
	void ResetHit();
	
	void DecreaseHp();
	void DestroyTimer();
	void DestroyEnemy();

	void SetDest(Enemy*);
	Coordiante* GetDest() { return &m_dstCoord; };
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UFUNCTION()
	void OnHandOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
private:
	UPROPERTY(VisibleDefaultsOnly, Category = Widget)
	TSubclassOf<UEnemyWidget> WidgetClass;

	UPROPERTY(VisibleDefaultsOnly, Category = Widget)
	UWidgetComponent* m_pWidgetComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* m_pSkeletalMeshComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = Collision)
	USphereComponent* m_pSphereComponent;

	UEnemyWidget* m_pWidget;

	FTimerHandle m_hTimer;

	bool m_bAttacking;
	bool m_bAttackSucceded;
	bool m_bGotHit;
	bool m_bDead;
	bool m_bDestroySet;

	int m_Hp;

	Coordiante m_dstCoord;
};
