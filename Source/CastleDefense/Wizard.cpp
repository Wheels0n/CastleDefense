// Fill out your copyright notice in the Description page of Project Settings.


#include "Wizard.h"

// Sets default values
AWizard::AWizard()
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

	Tags.Add(FName(TEXT("Wizard")));
}


// Called when the game starts or when spawned
void AWizard::BeginPlay()
{
	Super::BeginPlay();
	
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

void AWizard::StartJump()
{
	bPressedJump = true;
}

void AWizard::StopJump()
{
	bPressedJump = false;
}

