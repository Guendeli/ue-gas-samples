// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AG_WallRunAbility.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GASCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Tasks/AG_Task_TickWallRun.h"

UAG_WallRunAbility::UAG_WallRunAbility()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UAG_WallRunAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	ACharacter* CharacterAvatar = Cast<ACharacter>(ActorInfo->AvatarActor);
	UCapsuleComponent* CapsuleComponent = CharacterAvatar->GetCapsuleComponent();

	CapsuleComponent->OnComponentHit.AddDynamic(this, &UAG_WallRunAbility::OnCapsuleComponentHit);
}

void UAG_WallRunAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	if (ActorInfo)
	{
		ACharacter* CharacterAvatar = Cast<ACharacter>(ActorInfo->AvatarActor);
		UCapsuleComponent* CapsuleComponent = CharacterAvatar->GetCapsuleComponent();

		CapsuleComponent->OnComponentHit.RemoveDynamic(this, &UAG_WallRunAbility::OnCapsuleComponentHit);
	}

	Super::OnRemoveAbility(ActorInfo, Spec);
}

bool UAG_WallRunAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	AGASCharacter* CharacterAvatar = GetCharacterFromActorInfo();

	return CharacterAvatar && !CharacterAvatar->GetCharacterMovement()->IsMovingOnGround();
}

void UAG_WallRunAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	WallRunTickTask = UAG_Task_TickWallRun::CreateWallRunTask(this, Cast<ACharacter>(GetAvatarActorFromActorInfo()), Cast<UCharacterMovementComponent>(ActorInfo->MovementComponent), WallRun_TraceObjectTypes);

	WallRunTickTask->OnFinished.AddDynamic(this, &UAG_WallRunAbility::K2_EndAbility);
	WallRunTickTask->OnWallSideDetermened.AddDynamic(this, &UAG_WallRunAbility::OnWallSideDetermened);

	WallRunTickTask->ReadyForActivation();
}

void UAG_WallRunAbility::OnCapsuleComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->TryActivateAbility(GetCurrentAbilitySpec()->Handle);
	}
}

void UAG_WallRunAbility::OnWallSideDetermened(bool bLeftSide)
{
	AGASCharacter* CharacterAvatar = GetCharacterFromActorInfo();
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();

	FGameplayEffectContextHandle EffectContextHandle = AbilitySystemComponent->MakeEffectContext();

	if (bLeftSide)
	{
		CharacterAvatar->ApplyGameplayEffectToSelf(WallRunLeftSideEffectClass, EffectContextHandle);
	}
	else
	{
		CharacterAvatar->ApplyGameplayEffectToSelf(WallRunRightSideEffectClass, EffectContextHandle);
	}
}

void UAG_WallRunAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (IsValid(WallRunTickTask))
	{
		WallRunTickTask->EndTask();
	}

	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveActiveGameplayEffectBySourceEffect(WallRunLeftSideEffectClass, AbilitySystemComponent);
		AbilitySystemComponent->RemoveActiveGameplayEffectBySourceEffect(WallRunRightSideEffectClass, AbilitySystemComponent);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
