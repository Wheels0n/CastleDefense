// Fill out your copyright notice in the Description page of Project Settings.


#include "SkeletonEnemy.h"
#include "Wizard.h"
#include "EnemyAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
// Sets default values
ASkeletonEnemy::ASkeletonEnemy()
	: m_Hp(0), m_bAttacking(false), m_bHit(false)
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

		m_pSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
		m_pSphereComponent->SetupAttachment(m_pSkeletalMeshComponent, FName(TEXT("SKT_Sword")));
		m_pSphereComponent->SetCollisionProfileName(TEXT("NoCollision"));
		m_pSphereComponent->SetGenerateOverlapEvents(true);
	}
	static ConstructorHelpers::FClassFinder<AEnemyAIController> enemyAIController(TEXT("/Script/CoreUObject.Class'/Script/CastleDefense.EnemyAIController'"));
	AIControllerClass = enemyAIController.Class;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}


void ASkeletonEnemy::StartAttack()
{
	m_bAttacking = true; 

	UCharacterMovementComponent* pMoveComp = GetCharacterMovement();
	pMoveComp->MaxWalkSpeed = 0.0f;
	m_pSphereComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void ASkeletonEnemy::StopAttack()
{
	m_bAttacking = false; 
	m_bHit = false;	
	m_pSphereComponent->SetCollisionProfileName(TEXT("NoCollision"));
};

// Called when the game starts or when spawned
void ASkeletonEnemy::BeginPlay()
{
	Super::BeginPlay();
	m_pSphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ASkeletonEnemy::OnHandOverlap);
}

// Called every frame
void ASkeletonEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FString bAttackignStr = FString::FromInt((int32)m_bAttacking);
	GEngine->AddOnScreenDebugMessage(-2, 1.0f, FColor::Green, bAttackignStr);
}

// Called to bind functionality to input
void ASkeletonEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ASkeletonEnemy::OnHandOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (m_bAttacking&& m_bHit==false)
	{
		GEngine->AddOnScreenDebugMessage(-6, 1.0f, FColor::Yellow, TEXT("OnOverlap"));
		GEngine->AddOnScreenDebugMessage(-7, 1.0f, FColor::Yellow, TEXT("Attacking"));
		if (OtherActor->IsA(AWizard::StaticClass()))
		{
			m_bHit = true;
			AWizard* pWizard = Cast<AWizard>(OtherActor);
			pWizard->DecreaseHp();
		}
	}
	
}

