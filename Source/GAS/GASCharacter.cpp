// Copyright Epic Games, Inc. All Rights Reserved.

#include "GASCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Net/UnrealNetwork.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/ActorComponents/AG_CharacterMovementComponent.h"
#include "AbilitySystem/Attributes/AG_AttributeSetBase.h"
#include "AbilitySystem/Components/AG_AbilitySystemComponentBase.h"
#include "DataAssets/CharacterDataAsset.h"
#include "GAS/AbilitySystem/ActorComponents/AG_FootstepsComponent.h"
#include "AbilitySystem/ActorComponents/AG_CharacterMovementComponent.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AGASCharacter
AGASCharacter::AGASCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UAG_CharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
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
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

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
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	// Abilities
	AbilitySystemComponent = CreateDefaultSubobject<UAG_AbilitySystemComponentBase>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UAG_AttributeSetBase>(TEXT("AttributeSet"));

	// Footsteps

	FootstepsComponent = CreateDefaultSubobject<UAG_FootstepsComponent>(TEXT("FootstepsComponent"));
}
UAbilitySystemComponent* AGASCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AGASCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if(CharacterDataAsset != nullptr)
	{
		SetCharacterData(CharacterDataAsset->CharacterData);
	}
}

bool AGASCharacter::ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> Effect,
                                              FGameplayEffectContextHandle InEffectContext)
{
	if(Effect.Get() == nullptr) return false;

	FGameplayEffectSpecHandle specHandle = AbilitySystemComponent->MakeOutgoingSpec(Effect, 1, InEffectContext);
	if(specHandle.IsValid())
	{
		FActiveGameplayEffectHandle activeGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*specHandle.Data.Get());

		return activeGEHandle.WasSuccessfullyApplied();
	}
	return false;
}

void AGASCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	APlayerController* pc = Cast<APlayerController>(GetController());
	if(pc)
	{
		UEnhancedInputLocalPlayerSubsystem* subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(pc->GetLocalPlayer());
		subsystem->ClearAllMappings();
		subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}

void AGASCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	if(AbilitySystemComponent == nullptr)
		return;

	AbilitySystemComponent->RemoveActiveEffectsWithTags(InAirTags);
}


void AGASCharacter::GiveAbilities()
{
	if(HasAuthority() && AbilitySystemComponent)
	{
		for(TSubclassOf<UGameplayAbility> defaultAbility : CharacterData.Abilities)
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(defaultAbility));
		}
	}
}

void AGASCharacter::ApplyStartupEffects()
{
	if(HasAuthority() && AbilitySystemComponent)
	{
		FGameplayEffectContextHandle effectContext = AbilitySystemComponent->MakeEffectContext();
		effectContext.AddSourceObject(this);

		for(TSubclassOf<UGameplayEffect> characterEffect : CharacterData.Effects)
		{
			ApplyGameplayEffectToSelf(characterEffect, effectContext);
		}
	}
}

void AGASCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	GiveAbilities();
	ApplyStartupEffects();
}

void AGASCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

void AGASCharacter::BeginPlay()
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

FCharacterData AGASCharacter::GetCharacterData() const
{
	return CharacterData;
}

void AGASCharacter::SetCharacterData(const FCharacterData inCharacterData)
{
	CharacterData = inCharacterData;
	InitFromCharacterData(CharacterData);
}

UAG_FootstepsComponent* AGASCharacter::GetFootstepsComponent() const
{
	return FootstepsComponent;
}

void AGASCharacter::OnRep_CharacterData()
{
	InitFromCharacterData(CharacterData, true);
}

void AGASCharacter::InitFromCharacterData(const FCharacterData inCharacterData, bool bFromApplication)
{
}

//////////////////////////////////////////////////////////////////////////
// Input

void AGASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AGASCharacter::OnJumpStarted);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AGASCharacter::OnJumpEnded);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGASCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGASCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AGASCharacter::Move(const FInputActionValue& Value)
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

void AGASCharacter::Look(const FInputActionValue& Value)
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

void AGASCharacter::OnJumpStarted()
{
	FGameplayEventData payload;
	payload.Instigator = this;
	payload.EventTag = JumpEventTag;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, JumpEventTag, payload);
}

void AGASCharacter::OnJumpEnded()
{
}

void AGASCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGASCharacter, CharacterData);
}
