// Fill out your copyright notice in the Description page of Project Settings.

#include "Wizard.h"
#include "Blueprint/UserWidget.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CastleDefenceGameMode.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Animation/Animinstance.h"
#include "WizardWidget.h"
#include "SkeletonEnemy.h"
#include "Weapon.h"
// Sets default values
AWizard::AWizard()
	:m_Hp(100), m_bDead(false), m_bAttacking(false), m_bGotHit(false), m_bDestroySet(false)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	m_pSkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WizardMesh"));
	check(m_pSkeletalMeshComponent != nullptr);
	m_pSkeletalMeshComponent->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> wizardAsset(TEXT("/Script/Engine.SkeletalMesh'/Game/BattleWizardPolyart/Meshes/WizardSM.WizardSM'"));
	if (wizardAsset.Succeeded())
	{
		m_pSkeletalMeshComponent->SetSkeletalMesh(wizardAsset.Object);
		m_pSkeletalMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -95.0f));
		m_pSkeletalMeshComponent->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

		static ConstructorHelpers::FObjectFinder<UAnimBlueprint> wizardAnimBP
		(TEXT("/ Script / Engine.AnimBlueprint'/Game/BattleWizardPolyart/Animations/WizardAnImBP.WizardAnImBP'"));
		m_pSkeletalMeshComponent->SetAnimInstanceClass(wizardAnimBP.Object->GeneratedClass);
	}
	
	m_pCamComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	check(m_pCamComponent != nullptr);
	m_pCamComponent->SetupAttachment(RootComponent);
	m_pCamComponent->SetRelativeLocation(FVector(-250.0f, 0.0f, 0.0f));
	m_pCamComponent->bUsePawnControlRotation = true;

	UCharacterMovementComponent* pCharacterMovement = GetCharacterMovement();
	pCharacterMovement->MaxWalkSpeed = 150.0f;


	static ConstructorHelpers::FClassFinder<UUserWidget> wizardUIAsset(TEXT("UserWidget'/Game/WizardUI.WizardUI_C'"));
	if (wizardUIAsset.Succeeded())
	{
		UE_LOG(LogTemp, Display, TEXT("GotBluePrintUIClass"));
		WidgetClass = wizardUIAsset.Class;
	}

	Tags.Add(FName(TEXT("Wizard")));
}


// Called when the game starts or when spawned
void AWizard::BeginPlay()
{
	Super::BeginPlay();

	UWorld* pWorld = GetWorld();
	m_pWeapon = pWorld->SpawnActor(AWeapon::StaticClass());
	m_pWeapon->AttachToComponent(m_pSkeletalMeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale, FName(TEXT("WeaponSocket")));


	if (WidgetClass!=nullptr)
	{
		AController* pController = GetController();
		APlayerController* pPlayerController = Cast<APlayerController>(pController);
		m_pWidget = CreateWidget<UWizardWidget>(pPlayerController, WidgetClass);
		m_pWidget->SetHp(m_Hp);
		m_pWidget->AddToViewport();
	}
}

void AWizard::Destroyed()
{

	AController* pController = GetController();

	Super::Destroyed();

	UWorld* pWorld = GetWorld();
	if (pWorld)
	{
		AGameModeBase* pGameMode = pWorld->GetAuthGameMode();
		ACastleDefenceGameMode* pCastleDefenceGameMode = Cast<ACastleDefenceGameMode>(pGameMode);
		const FOnPlayerDiedSignature& delegateRef = pCastleDefenceGameMode->GetOnPlayerDied();
		delegateRef.Broadcast(pController);
		UE_LOG(LogTemp, Display, TEXT("Destroyed"));
	}
	

}

// Called every frame
void AWizard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AWizard::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AWizard::StartJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AWizard::StopJump);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AWizard::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AWizard::StopSprint);
	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AWizard::StartAttack);
	PlayerInputComponent->BindAction("Attack", IE_Released, this, &AWizard::StopAttack);

	PlayerInputComponent->BindAxis("Move_Forward", this, &AWizard::MoveForward);
	PlayerInputComponent->BindAxis("Move_Right", this, &AWizard::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AWizard::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &AWizard::AddControllerPitchInput);
}

void AWizard::MoveForward(float value)
{
	FVector dir = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::X);
	AddMovementInput(dir, value);
}

void AWizard::MoveRight(float value)
{
	FVector dir = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::Y);
	AddMovementInput(dir, value);
}

void AWizard::StartAttack()
{
	if (!m_bGotHit)
	{
		m_bAttacking = true;
	}
}

void AWizard::StopAttack()
{
	m_bAttacking = false;
}

void AWizard::StartSprint()
{
	UCharacterMovementComponent* pCharacterMovement = GetCharacterMovement();
	pCharacterMovement->MaxWalkSpeed = 500.0f;
}

void AWizard::StopSprint()
{
	UCharacterMovementComponent* pCharacterMovement = GetCharacterMovement();
	pCharacterMovement->MaxWalkSpeed = 150.0f;
}

void AWizard::StartJump()
{
	bPressedJump = true;
}

void AWizard::StopJump()
{
	bPressedJump = false;
}

void AWizard::CheckPlayerAttack()
{
	UWorld* pWorld = GetWorld();
	if (pWorld)
	{
		FHitResult hitResult;
		FVector start = GetActorLocation();
		FVector end = start + GetActorForwardVector() * 500.f;
		FCollisionObjectQueryParams queryObjParams;
		queryObjParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Pawn);
		FCollisionQueryParams queryParams;
		queryParams.AddIgnoredActor(this);
		pWorld->LineTraceSingleByObjectType(hitResult, start, end, queryObjParams, queryParams);
		DrawDebugLine(pWorld, start, end, FColor::Red, false, 5.0f);
		if (hitResult.bBlockingHit)
		{
			AActor* pOther = hitResult.GetActor();
			if (pOther&&pOther->IsA(ASkeletonEnemy::StaticClass()))
			{
				UE_LOG(LogTemp, Display, TEXT("EnemyGotHit"));
				ASkeletonEnemy* pEnemy = Cast<ASkeletonEnemy>(pOther);
				pEnemy->DecreaseHp();
			}
		}
	}
	}
	

void AWizard::DecreaseHp()
{
	{
		FString HPStr = FString::FromInt(m_Hp);
		UE_LOG(LogTemp, Display, TEXT("%s"), *HPStr);
		m_Hp -= 10;
		if (m_pWidget)
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

void AWizard::DestroyTimer()
{
	m_bDestroySet = true;
	AController* pController = GetController();
	APlayerController* pPlayerController = Cast<APlayerController>(pController);
	DisableInput(pPlayerController);

	UWorld* pWorld = GetWorld();
	if (pWorld)
	{
		FTimerManager& timerManager = pWorld->GetTimerManager();
		timerManager.SetTimer(m_hTimer, this, &AWizard::DestroyPlayer, 4.0f);
	}
}

void AWizard::DestroyPlayer()
{
	Destroy();
	m_pWeapon->Destroy();
}

