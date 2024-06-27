// Fill out your copyright notice in the Description page of Project Settings.


#include "SkeletonEnemy.h"
#include "EnemyAIController.h"
// Sets default values
ASkeletonEnemy::ASkeletonEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	m_pSkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("EnemyMesh"));
	check(m_pSkeletalMeshComponent != nullptr);
	m_pSkeletalMeshComponent->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> enemyMeshAsset(TEXT("/Script/Engine.SkeletalMesh'/Game/UndeadPack/SkeletonEnemy/Mesh/SK_Skeleton.SK_Skeleton'"));
	if (enemyMeshAsset.Succeeded())
	{
		m_pSkeletalMeshComponent->SetSkeletalMesh(enemyMeshAsset.Object);
		m_pSkeletalMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
		m_pSkeletalMeshComponent->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
		static ConstructorHelpers::FObjectFinder<UAnimBlueprint> enemyAnimBP
		(TEXT("/Script/Engine.AnimBlueprint'/Game/UndeadPack/SkeletonEnemy/Animations/Enemy_AnimBP.Enemy_AnimBP'"));
		m_pSkeletalMeshComponent->SetAnimInstanceClass(enemyAnimBP.Object->GeneratedClass);
	}
	static ConstructorHelpers::FClassFinder<AEnemyAIController> enemyAIController(TEXT("/Script/CoreUObject.Class'/Script/CastleDefense.EnemyAIController'"));
	AIControllerClass = enemyAIController.Class;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

// Called when the game starts or when spawned
void ASkeletonEnemy::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ASkeletonEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASkeletonEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

