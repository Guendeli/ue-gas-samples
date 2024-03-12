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
	if (!Super::CommitCheck(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags))
	{
		return false;
	}

	AGASCharacter* Character = GetCharacterFromActorInfo();

	if (!IsValid(Character))
	{
		return false;
	}

	const FVector StartLocation = Character->GetActorLocation();
	const FVector ForwardVector = Character->GetActorForwardVector();
	const FVector UpVector = Character->GetActorUpVector();

	TArray<AActor*> ActorsToIgnore = {Character};

	static const auto CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("ShowDebugTraversal"));
	const bool bShowTraversal = CVar->GetInt() > 0;

	EDrawDebugTrace::Type DebugDrawType = bShowTraversal ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

	bool bJumpToLocationSet = false;

	int32 JumpToLocationIdx = INDEX_NONE;

	int i = 0;

	FHitResult TraceHit;

	float MaxJumpDistance = HorizontalTraceLenght;

	for (;i < HorizontalTraceCount; ++i)
	{
		const FVector TraceStart = StartLocation + i * UpVector * HorizontalTraceStep;
		const FVector TraceEnd = TraceStart + ForwardVector * HorizontalTraceLenght;

		if (UKismetSystemLibrary::SphereTraceSingleForObjects(this, TraceStart, TraceEnd, HorizontalTraceRadius, TraceObjectTypes, true, ActorsToIgnore, DebugDrawType, TraceHit, true))
		{
			if (JumpToLocationIdx == INDEX_NONE && (i < HorizontalTraceCount - 1))
			{
				JumpToLocationIdx = i;
				JumpToLocation = TraceHit.Location;
			}
			else if (JumpToLocationIdx == (i - 1))
			{
				MaxJumpDistance = FVector::Dist2D(TraceHit.Location, TraceStart);
				break;
			}
		}
		else
		{
			if (JumpToLocationIdx != INDEX_NONE)
			{
				break;
			}
		}
	}

	if (JumpToLocationIdx == INDEX_NONE)
	{
		return false;
	}

	const float DistanceToJumpTo = FVector::Dist2D(StartLocation, JumpToLocation);

	const float MaxVerticalTraceDistance = MaxJumpDistance - DistanceToJumpTo;

	if (MaxVerticalTraceDistance < 0)
	{
		return false;
	}

	if (i == HorizontalTraceCount)
	{
		i = HorizontalTraceCount - 1;
	}

	const float VerticalTraceLength = FMath::Abs(JumpToLocation.Z - (StartLocation + i * UpVector * HorizontalTraceStep).Z);

	FVector VerticalStartLocation = JumpToLocation + UpVector * VerticalTraceLength;

	i = 0;

	const float VerticalTraceCount = MaxVerticalTraceDistance / VerticalTraceStep;

	bool bJumpOverLocationSet = false;

	for (; i <= VerticalTraceCount; ++i)
	{
		const FVector TraceStart = VerticalStartLocation + i * ForwardVector * VerticalTraceStep;
		const FVector TraceEnd = TraceStart + UpVector * VerticalTraceLength * -1;

		if (UKismetSystemLibrary::SphereTraceSingleForObjects(this, TraceStart, TraceEnd, HorizontalTraceRadius, TraceObjectTypes, true, ActorsToIgnore, DebugDrawType, TraceHit, true))
		{
			JumpOverLocation = TraceHit.ImpactPoint;

			if (i == 0)
			{
				JumpToLocation = JumpOverLocation;
			}
		}
		else if(i != 0)
		{
			bJumpOverLocationSet = true;
			break;
		}
	}

	if (!bJumpOverLocationSet)
	{
		return false;
	}

	const FVector TraceStart = JumpOverLocation + ForwardVector * VerticalTraceStep;

	if (UKismetSystemLibrary::SphereTraceSingleForObjects(this, TraceStart, JumpOverLocation, HorizontalTraceRadius, TraceObjectTypes, true, ActorsToIgnore, DebugDrawType, TraceHit, true))
	{
		JumpOverLocation = TraceHit.ImpactPoint;
	}

	if (bShowTraversal)
	{
		DrawDebugSphere(GetWorld(), JumpToLocation, 15, 16, FColor::White, false, 7);
		DrawDebugSphere(GetWorld(), JumpOverLocation, 15, 16, FColor::White, false, 7);
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
