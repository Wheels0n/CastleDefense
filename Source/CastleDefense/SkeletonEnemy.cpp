// Fill out your copyright notice in the Description page of Project Settings.


#include "SkeletonEnemy.h"
#include "Blueprint/UserWidget.h"
#include "Wizard.h"
#include "EnemyAIController.h"
#include "CastleDefenseGameState.h"
#include "Components/SphereComponent.h"
#include "EnemyWidget.h" 
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
// Sets default values
ASkeletonEnemy::ASkeletonEnemy()
	: m_Hp(100), m_bAttacking(false), m_bAttackSucceded(false), m_bGotHit(false), m_bDead(false), m_bDestroySet(false),
	m_pWidget(nullptr), m_pWidgetComponent(nullptr)
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


	m_pWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("EnemyHPBar"));
	m_pWidgetComponent->SetupAttachment(m_pSkeletalMeshComponent);
	static ConstructorHelpers::FClassFinder<UUserWidget> enemyUIAsset(TEXT("UserWidget'/Game/EnemyUI.EnemyUI_C'"));
	if (enemyUIAsset.Succeeded())
	{
		UE_LOG(LogTemp, Display, TEXT("GotBluePrintUIClass"));
		WidgetClass = enemyUIAsset.Class;
	}
}


void ASkeletonEnemy::StartAttack()
{
	if (!m_bGotHit)
	{
		m_bAttacking = true;

		UCharacterMovementComponent* pMoveComp = GetCharacterMovement();
		pMoveComp->MaxWalkSpeed = 0.0f;
		m_pSphereComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	}
}

void ASkeletonEnemy::StopAttack()
{
	m_bAttacking = false; 
	m_bAttackSucceded = false;
	m_pSphereComponent->SetCollisionProfileName(TEXT("NoCollision"));
};

// Called when the game starts or when spawned
void ASkeletonEnemy::BeginPlay()
{
	Super::BeginPlay();
	m_pSphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ASkeletonEnemy::OnHandOverlap);

	if (WidgetClass != nullptr)
	{
		UWorld* pWorld = GetWorld();
		if (pWorld!=nullptr)
		{	
			m_pWidget = CreateWidget<UEnemyWidget>(pWorld, WidgetClass);
			if (m_pWidget != nullptr)
			{
				m_pWidget->SetHp(m_Hp);
				m_pWidgetComponent->SetWidget(m_pWidget);
				m_pWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
				m_pWidgetComponent->SetTwoSided(true);
				m_pWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));
			}
		}
		
	}
}

void ASkeletonEnemy::Destroyed()
{
	AController* pController = GetController();

	UWorld* pWorld = GetWorld();
	AGameStateBase* pGameBase = pWorld->GetGameState();
	if (pGameBase)
	{
		//ACastleDefenseGameState* pCastleDefenseGameState = Cast<ACastleDefenseGameState>(pGameBase);
		//pCastleDefenseGameState->SetDeleteEnemy(this);
	}
	Super::Destroyed();
	UE_LOG(LogTemp, Display, TEXT("EnemyDestroyed"));
}

// Called every frame
void ASkeletonEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FString bAttackignStr = FString::FromInt((int32)m_bAttacking);
	UE_LOG(LogTemp, Display, TEXT("%s"), *bAttackignStr);

}

// Called to bind functionality to input
void ASkeletonEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ASkeletonEnemy::OnHandOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (m_bAttacking&& m_bAttackSucceded ==false)
	{
		UE_LOG(LogTemp, Display, TEXT("OnOverlap"));
		UE_LOG(LogTemp, Display, TEXT("Attacking"));
		if (OtherActor->IsA(AWizard::StaticClass()))
		{
			m_bAttackSucceded = true;
			AWizard* pWizard = Cast<AWizard>(OtherActor);
			pWizard->DecreaseHp();
		}
	}
	
}

void ASkeletonEnemy::SetHit()
{
	AController* pController = GetController();
	AEnemyAIController* pAIController = Cast<AEnemyAIController>(pController);
	pAIController->SetGotHit(m_bGotHit);
}

void ASkeletonEnemy::ResetHit()
{
	m_bGotHit = false;
	SetHit();
}

void ASkeletonEnemy::DecreaseHp()
{
	if (!m_bGotHit)
	{
		m_bGotHit = true;
		SetHit();
	}

	{
		FString HPStr = FString::FromInt(m_Hp);
		UE_LOG(LogTemp, Display, TEXT("%s"), *HPStr);
		m_Hp -= 50;
		if (m_pWidget != nullptr)
		{
			m_pWidget->SetHp(m_Hp);
		}
		
		if (m_Hp <= 0)
		{
			m_bDead = true;
		}
		else
		{
			m_bGotHit = true;
		}
	}
}

void ASkeletonEnemy::DestroyTimer()
{
	m_bDestroySet = true;
	AController* pController = GetController();
	AEnemyAIController* pAIController = Cast<AEnemyAIController>(pController);
	pAIController->StopBehaviorTree();

	UWorld* pWolrd = GetWorld();
	if (pWolrd)
	{
		FTimerManager& timerManager = pWolrd->GetTimerManager();
		timerManager.SetTimer(m_hTimer, this, &ASkeletonEnemy::DestroyEnemy, 4.0f);
	}
	
}

void ASkeletonEnemy::DestroyEnemy()
{
	Destroy();
}
