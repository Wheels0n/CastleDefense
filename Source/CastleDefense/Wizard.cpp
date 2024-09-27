// Fill out your copyright notice in the Description page of Project Settings.

#include "Wizard.h"
#include "Blueprint/UserWidget.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Animation/Animinstance.h"
#include "CastleDefenceGameMode.h"
#include "CastleDefenseGameState.h"
#include "CastleDefenseGameInstance.h"
#include "Weapon.h"
#include "WizardWidget.h"
#include "ChatWidget.h"
#include "ClientSession.h"
#include "SkeletonEnemy.h"
// Sets default values
AWizard::AWizard()
	:m_Hp(100), m_bDead(false), m_bAttacking(false), m_bGotHit(false), m_bDestroySet(false), m_bMoveStateChanged(false), m_broadCastTime(0.2f)
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
	pCharacterMovement->bRunPhysicsWithNoController = true;

	static ConstructorHelpers::FClassFinder<UUserWidget> playerWidgetAsset(TEXT("UserWidget'/Game/WizardUI.WizardUI_C'"));
	if (playerWidgetAsset.Succeeded())
	{
		UE_LOG(LogTemp, Display, TEXT("GotUserUIClass"));
		PlayerWidgetClass = playerWidgetAsset.Class;
	}
	static ConstructorHelpers::FClassFinder<UUserWidget> pauseWidgetAsset(TEXT("UserWidget'/Game/PauseUI.PauseUI_C'"));
	if (pauseWidgetAsset.Succeeded())
	{
		UE_LOG(LogTemp, Display, TEXT("GotPauseUIClass"));
		pauseWidgetClass = pauseWidgetAsset.Class;
	}

	static ConstructorHelpers::FClassFinder<UChatWidget> chatWidgetAsset(TEXT("UserWidget'/Game/ChatUI.ChatUI_C'"));
	if (chatWidgetAsset.Succeeded())
	{
		UE_LOG(LogTemp, Display, TEXT("GotChatUIClass"));
		chatWidgetClass = chatWidgetAsset.Class;
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

	FVector curPos = GetActorLocation();
	m_curCoord.set_x(curPos.X);
	m_curCoord.set_y(curPos.Y);
	m_curCoord.set_z(curPos.Z);

	GEngine->AddOnScreenDebugMessage(INDEX_NONE, 23.0f, FColor::Yellow, FString::Printf(TEXT("Client Spawn: %f %f %f"), curPos.X, curPos.Y, curPos.Z));

	m_dstCoord.set_x(curPos.X);
	m_dstCoord.set_y(curPos.Y);
	m_dstCoord.set_z(curPos.Z);

	FRotator curRot = GetActorRotation();
	m_curRot.set_x(curRot.Roll);
	m_curRot.set_y(curRot.Yaw);
	m_curRot.set_z(curRot.Pitch);

	m_dstRot.set_x(curRot.Roll);
	m_dstRot.set_y(curRot.Yaw);
	m_dstRot.set_z(curRot.Pitch);

	m_curMoveState = IDLE;

}

void AWizard::Destroyed()
{

	AController* pController = GetController();

	Super::Destroyed();

	UWorld* pWorld = GetWorld();
	if (pController&&pWorld)
	{
		AGameModeBase* pGameMode = pWorld->GetAuthGameMode();
		ACastleDefenceGameMode* pCastleDefenceGameMode = Cast<ACastleDefenceGameMode>(pGameMode);
		const FOnPlayerDiedSignature& delegateRef = pCastleDefenceGameMode->GetOnPlayerDied();
		delegateRef.Broadcast(pController);
		UE_LOG(LogTemp, Display, TEXT("Destroyed"));
	}
	
	if (m_pWeapon)
	{
		m_pWeapon->Destroy();
	}

}

// Called every frame
void AWizard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AController* pController = GetController();

	if (pController==nullptr)
	{
		FRotator dstRot(m_dstRot.x(), m_dstRot.y(), m_dstRot.z());
		SetActorRotation(dstRot);

		if(m_curMoveState != IDLE)
		{	
			FVector dstLoc(m_dstCoord.x(), m_dstCoord.y(), m_dstCoord.z());
			FVector srcLoc(m_curCoord.x(), m_curCoord.y(), m_curCoord.z());

			FVector distVec = dstLoc - srcLoc;
			//distVec.Normalize();

			float walkSpeed = m_curMoveState == SPRINT ? 500.0f : 150.f;
			UCharacterMovementComponent* pCharacterMovement = GetCharacterMovement();
			pCharacterMovement->MaxWalkSpeed = walkSpeed;

			FVector newLoc = srcLoc + distVec*DeltaTime;
			newLoc.X = dstLoc.X < srcLoc.X ? fmax(newLoc.X, dstLoc.X) : fmin(newLoc.X, dstLoc.X);
			newLoc.Y = dstLoc.Y < srcLoc.Y ? fmax(newLoc.Y, dstLoc.Y) : fmin(newLoc.Y, dstLoc.Y);
			newLoc.Z = dstLoc.Z < srcLoc.Z ? fmax(newLoc.Z, dstLoc.Z) : fmin(newLoc.Z, dstLoc.Z);

			m_curCoord.set_x(newLoc.X);
			m_curCoord.set_y(newLoc.Y);
			m_curCoord.set_z(newLoc.Z);

			AddMovementInput(newLoc - srcLoc);
			if (m_curMoveState == JUMP)
			{
				StartJump();
			}
			else
			{
				StopJump();
			}
		}
		else
		{
			//자꾸 예전 방향으로 가서 처리
			m_curCoord = m_dstCoord;
			FVector dstLoc(m_dstCoord.x(), m_dstCoord.y(), m_dstCoord.z());
			SetActorLocation(dstLoc);
		}

	}
	else
	{	

		APlayerController* pPlayerController = Cast<APlayerController>(pController);
		FRotator curRot = GetActorRotation();
		if ((int)curRot.Yaw != (int)m_curRot.y())
		{	
			m_bMoveStateChanged=true;
		}
		
		if (pPlayerController->WasInputKeyJustPressed(EKeys::SpaceBar))
		{
			m_curMoveState = JUMP;
			m_bMoveStateChanged = true;
		}
		//멈췄다가 움직일때
		else if (pPlayerController->WasInputKeyJustPressed(EKeys::W) || pPlayerController->WasInputKeyJustPressed(EKeys::S) ||
			pPlayerController->WasInputKeyJustPressed(EKeys::A) || pPlayerController->WasInputKeyJustPressed(EKeys::D))
		{
			m_curMoveState = pPlayerController->IsInputKeyDown(EKeys::LeftShift) ? SPRINT: WALK;
			m_bMoveStateChanged = true;
		}
		//걷다가 달리는 경우
		else if (m_curMoveState==WALK&& pPlayerController->WasInputKeyJustPressed(EKeys::LeftShift))
		{
			m_curMoveState = SPRINT;
			m_bMoveStateChanged = true;
		}
		else if (!pPlayerController->IsInputKeyDown(EKeys::AnyKey)&&m_curMoveState!=IDLE)
		{
			m_curMoveState = IDLE;
			m_bMoveStateChanged = true;
		}

		if (m_bMoveStateChanged)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.0f, FColor::Red,
				FString::Printf(TEXT("MoveStateChanged %d"), m_curMoveState));
		}

		m_broadCastTime -= DeltaTime;
		if (m_broadCastTime<=0.0f|| m_bMoveStateChanged)
		{
			m_broadCastTime = 0.2f;
			BrodcastPos();
		}
				
	}
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
	PlayerInputComponent->BindAction("Pause", IE_Pressed, this, &AWizard::OpenPauseMenu);
	PlayerInputComponent->BindAction("Chat", IE_Pressed, this, &AWizard::FocusOnChat);

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

void AWizard::OpenPauseMenu()
{
	m_pPauseWidget->AddToViewport();
}

void AWizard::FocusOnChat()
{
	m_pChatWidget->FocusOnChat();
}

void AWizard::StartJump()
{
	bPressedJump = true;
}

void AWizard::StopJump()
{
	bPressedJump = false;
}

void AWizard::AddChat(std::string& msg)
{
	FString fstr(msg.c_str());
	m_pChatWidget->AddMessage(fstr);
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
				
				UWorld* pCurWorld = GetWorld();
				ACastleDefenseGameState* pGameState = pCurWorld->GetGameState<ACastleDefenseGameState>();
				int idx = pGameState->GetEnemyIndexByPtr(pEnemy);

				UCastleDefenseGameInstance* pGameInstance = pCurWorld->GetGameInstance<UCastleDefenseGameInstance>();
				TSharedPtr<ClientSession> pSession = pGameInstance->GetSession();
				pSession->SendC_Attack(idx);

			}	
		}
	}
}

void AWizard::LazyCreateWidget()
{
	if (PlayerWidgetClass != nullptr)
	{
		AController* pController = GetController();
		if (pController)
		{
			APlayerController* pPlayerController = Cast<APlayerController>(pController);
			m_pPlayerWidget = CreateWidget<UWizardWidget>(pPlayerController, PlayerWidgetClass);
			m_pPlayerWidget->SetHp(m_Hp);
			m_pPlayerWidget->AddToViewport();
		}

	}
	if (pauseWidgetClass != nullptr)
	{
		AController* pController = GetController();
		if (pController)
		{
			APlayerController* pPlayerController = Cast<APlayerController>(pController);
			m_pPauseWidget = CreateWidget<UUserWidget>(pPlayerController, pauseWidgetClass);
		}

	}

	if (chatWidgetClass != nullptr)
	{
		AController* pController = GetController();
		if (pController)
		{
			APlayerController* pPlayerController = Cast<APlayerController>(pController);
			m_pChatWidget = CreateWidget<UChatWidget>(pPlayerController, chatWidgetClass);
			m_pChatWidget->AddToViewport();
		}

	}
}
	

void AWizard::DecreaseHp()
{
	{
		FString HPStr = FString::FromInt(m_Hp);
		UE_LOG(LogTemp, Display, TEXT("%s"), *HPStr);
		m_Hp -= 10;
		if (m_pPlayerWidget)
		{
			m_pPlayerWidget->SetHp(m_Hp);
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

void AWizard::BrodcastPos()
{
	UWorld* pCurWorld = GetWorld();
	UCastleDefenseGameInstance* pGameInstance = pCurWorld->GetGameInstance<UCastleDefenseGameInstance>();
	TSharedPtr<ClientSession> pSession = pGameInstance->GetSession();

	FVector curPos = GetActorLocation();
	m_dstCoord.set_x(curPos.X);
	m_dstCoord.set_y(curPos.Y);
	m_dstCoord.set_z(curPos.Z);

	FRotator curRot = GetActorRotation();
	m_curRot.set_x(0);
	m_curRot.set_y(curRot.Yaw);
	m_curRot.set_z(0);
	
	m_bMoveStateChanged = false;
	pSession->SendC_Move(&m_dstCoord, &m_curRot, m_curMoveState, m_bAttacking);
}

void AWizard::SetNewDest(Player* pPlayer)
{
	m_dstCoord = pPlayer->coord();
	m_dstRot = pPlayer->rot();
	m_curMoveState = pPlayer->movestate();
	GEngine->AddOnScreenDebugMessage(INDEX_NONE, 23.0f, FColor::Yellow, FString::Printf(TEXT("CurState: %d"), m_curMoveState));
}
