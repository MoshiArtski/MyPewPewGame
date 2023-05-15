// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZaProjectCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "PlayerStats.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"




//////////////////////////////////////////////////////////////////////////
// AZaProjectCharacter

AZaProjectCharacter::AZaProjectCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++);
}

void AZaProjectCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

}

//////////////////////////////////////////////////////////////////////////
// Input

void AZaProjectCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AZaProjectCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AZaProjectCharacter::Look);
	
		// AutoAttack
		EnhancedInputComponent->BindAction(AutoAttackAction, ETriggerEvent::Triggered, this, &AZaProjectCharacter::AutoAttack);

		// Blocking
		EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Triggered, this, &AZaProjectCharacter::Block);

		// Abilities
		EnhancedInputComponent->BindAction(Ability1Action, ETriggerEvent::Triggered, this, &AZaProjectCharacter::Ability1);
		EnhancedInputComponent->BindAction(Ability2Action, ETriggerEvent::Triggered, this, &AZaProjectCharacter::Ability2);
		EnhancedInputComponent->BindAction(Ability3Action, ETriggerEvent::Triggered, this, &AZaProjectCharacter::Ability3);
		EnhancedInputComponent->BindAction(Ability4Action, ETriggerEvent::Triggered, this, &AZaProjectCharacter::Ability4);
	}
}

void AZaProjectCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AZaProjectCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AZaProjectCharacter::AutoAttack(const FInputActionValue& Value)
{
}

void AZaProjectCharacter::Block(const FInputActionValue& Value)
{
}

void AZaProjectCharacter::Ability1(const FInputActionValue& Value)
{
}

void AZaProjectCharacter::Ability2(const FInputActionValue& Value)
{
}

void AZaProjectCharacter::Ability3(const FInputActionValue& Value)
{
}

void AZaProjectCharacter::Ability4(const FInputActionValue& Value)
{
}

void AZaProjectCharacter::DisableCharacterInput()
{
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->SetComponentTickEnabled(false);
	}

	// Disable input for player controller
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		PlayerController->DisableInput(PlayerController);
	}
}

void AZaProjectCharacter::EnableCharacterInput()
{
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->SetComponentTickEnabled(true);
	}

	// Enable input for player controller
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		PlayerController->EnableInput(PlayerController);
	}
}


