// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AG_VaultOverAbility.h"
#include "GASCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AG_BaseMotionWarpingComponent.h"

UAG_VaultOverAbility::UAG_VaultOverAbility()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor; // TODO - Could be nonInstanced, just trying to showcase different approach
}

bool UAG_VaultOverAbility::CommitCheck(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	FGameplayTagContainer* OptionalRelevantTags)
{
	if(!Super::CommitCheck(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags)) return false;

	AGASCharacter* character = GetCharacterFromActorInfo();
	if(!IsValid(character)) return false;

	const FVector startLocation = character->GetActorLocation();
	const FVector forwardVector = character->GetActorForwardVector();
	const FVector upVector = character->GetActorUpVector();

	TArray<AActor*> actorsToIgnore = {character};

	// debug Only
	static const auto CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("ShowDebugTraversal"));
	const bool bIsDebug = CVar->GetInt() > 0;
	EDrawDebugTrace::Type debugDrawType = bIsDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

	bool bJumpToLocationSet = false;
	int32 JumpToLocationIdx = INDEX_NONE;
	
	int i = 0;
	FHitResult hitResult;
	float maxJumpDistance = HorizontalTraceLenght;

	for(i = 0;i < HorizontalTraceCount; i++)
	{
		const FVector traceStart = startLocation + i * upVector * HorizontalTraceStep;
		const FVector traceEnd = traceStart + forwardVector * HorizontalTraceLenght;

		bool hTrace = UKismetSystemLibrary::SphereTraceSingleForObjects(this,
			traceStart, traceEnd, HorizontalTraceRadius,
			TraceObjectTypes, true, actorsToIgnore,debugDrawType, hitResult,true);
		if(hTrace)
		{
			if(JumpToLocationIdx == INDEX_NONE &&
				(i <  HorizontalTraceCount - 1))
			{
				JumpToLocationIdx = i;
				JumpToLocation = hitResult.Location;
			} else if (JumpToLocationIdx == i -1)
			{
				maxJumpDistance = FVector::Dist2D(hitResult.Location, traceStart);
				break;
				
			}
		} else
		{
			if(JumpToLocationIdx != INDEX_NONE)
			{
				break;
			}
		}
	}

	if(JumpToLocationIdx == INDEX_NONE)
	{
		return false;
	}

	// VERTICAL TRACE
	const float distanceToJumpTo = FVector::Dist2D(startLocation, JumpToLocation);
	const float maxVerticalTraceDistance = maxJumpDistance - distanceToJumpTo;

	if(maxVerticalTraceDistance <= 0)
	{
		return false;
	}

	if(i == HorizontalTraceCount)
	{
		i = HorizontalTraceCount - 1;
	}

	const float verticalTraceLenght = FMath::Abs(JumpToLocation.Z - (startLocation + i * upVector * HorizontalTraceStep).Z);
	FVector verticalStartLocation = JumpToLocation + upVector * verticalTraceLenght;
	i = 0;
	const float verticalTraceCount = maxVerticalTraceDistance / VerticalTraceStep;
	bool bJumpOverLocationSet = false;
	for(i = 0; i < verticalTraceCount; i++)
	{
		const FVector traceStart = verticalStartLocation + i * forwardVector * VerticalTraceStep;
		const FVector traceEnd = traceStart + upVector * verticalTraceLenght * -1; // We trace down;

		bool vTrace = UKismetSystemLibrary::SphereTraceSingleForObjects(this,
			traceStart, traceEnd, VerticalTraceRadius,
			TraceObjectTypes, true, actorsToIgnore,debugDrawType, hitResult,true);
		if(vTrace)
		{
			JumpOverLocation = hitResult.ImpactPoint;
			if(i == 0)
			{
				JumpToLocation = JumpOverLocation;
			} else if (i != 0)
			{
				bJumpOverLocationSet = true;
				break;
			}
		}
	}

	if(bJumpOverLocationSet == false)
	{
		return false;
	}

	// final trace
	const FVector traceStart = JumpOverLocation + forwardVector * VerticalTraceStep;
	bool finalTrace = UKismetSystemLibrary::SphereTraceSingleForObjects(this,
			traceStart, JumpOverLocation, VerticalTraceRadius,
			TraceObjectTypes, true, actorsToIgnore,debugDrawType, hitResult,true);
	if(finalTrace)
	{
		JumpOverLocation = hitResult.ImpactPoint;
	}

	if(bIsDebug)
	{
		DrawDebugSphere(GetWorld(), JumpToLocation, 15,16,FColor::White, false, 7);
		DrawDebugSphere(GetWorld(), JumpOverLocation, 15,16,FColor::White, false, 7);
	}
	return true;
}

void UAG_VaultOverAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		K2_EndAbility();
		return;
	}

	AGASCharacter* character = GetCharacterFromActorInfo();
	
	UCharacterMovementComponent* moveComponent = character ? character->GetCharacterMovement() : nullptr;
	if(moveComponent)
	{
		moveComponent->SetMovementMode(MOVE_Flying);
	}

	UCapsuleComponent* capsuleComp = character ? character->GetCapsuleComponent() : nullptr;
	if(capsuleComp)
	{
		for(ECollisionChannel channel : CollisionChannelsToIgnore)
		{
			capsuleComp->SetCollisionResponseToChannel(channel, ECR_Ignore);
		}
	}

	UAG_BaseMotionWarpingComponent* motionWarpingComp = character ? character->GetAGMotionWarpingComponent() : nullptr;
	if(motionWarpingComp)
	{
		motionWarpingComp->AddOrUpdateWarpTargetFromLocationAndRotation(TEXT("JumpToLocation"), JumpToLocation, character->GetActorRotation());
		motionWarpingComp->AddOrUpdateWarpTargetFromLocationAndRotation(TEXT("JumpOverLocation"), JumpOverLocation, character->GetActorRotation());
	}

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, VaultMontage);
	MontageTask->OnBlendOut.AddDynamic(this, &UAG_VaultOverAbility::K2_EndAbility);
	MontageTask->OnCompleted.AddDynamic(this, &UAG_VaultOverAbility::K2_EndAbility);
	MontageTask->OnCancelled.AddDynamic(this, &UAG_VaultOverAbility::K2_EndAbility);
	MontageTask->OnInterrupted.AddDynamic(this, &UAG_VaultOverAbility::K2_EndAbility);
	MontageTask->ReadyForActivation();
}

void UAG_VaultOverAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if(IsValid(MontageTask))
	{
		MontageTask->EndTask();
	}

	AGASCharacter* character = GetCharacterFromActorInfo();
	UCapsuleComponent* capsuleComp = character ? character->GetCapsuleComponent() : nullptr;
	if(capsuleComp)
	{
		for(ECollisionChannel channel : CollisionChannelsToIgnore)
		{
			capsuleComp->SetCollisionResponseToChannel(channel, ECR_Block);
		}
	}
	
	UCharacterMovementComponent* moveComponent = character ? character->GetCharacterMovement() : nullptr;
	if(moveComponent && moveComponent->IsFlying())
	{
		moveComponent->SetMovementMode(MOVE_Falling);
	}

	UAG_BaseMotionWarpingComponent* motionWarpingComp = character ? character->GetAGMotionWarpingComponent() : nullptr;
	if(motionWarpingComp)
	{
		motionWarpingComp->RemoveWarpTarget(TEXT("JumpToLocation"));
		motionWarpingComp->RemoveWarpTarget(TEXT("JumpOverLocation"));
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
