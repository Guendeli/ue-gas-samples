// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AG_JumpAbility.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"

UAG_JumpAbility::UAG_JumpAbility()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
}

bool UAG_JumpAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if(!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	ACharacter* character = CastChecked<ACharacter>(ActorInfo->AvatarActor.Get(), ECastCheckedType::NullAllowed);
	
	return character->CanJump();
}

void UAG_JumpAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			return;
		}

		ACharacter * Character = CastChecked<ACharacter>(ActorInfo->AvatarActor.Get());
		Character->Jump();

		// Effects region

		UAbilitySystemComponent* abilityComponnent = ActorInfo->AbilitySystemComponent.Get();
		if(abilityComponnent == nullptr)
			return;

		FGameplayEffectContextHandle effectsContext = abilityComponnent->MakeEffectContext();
		FGameplayEffectSpecHandle specHandle = abilityComponnent->MakeOutgoingSpec(JumpEffect, 1, effectsContext);
		if(specHandle.IsValid())
		{
			FActiveGameplayEffectHandle activeGEHandle = abilityComponnent->ApplyGameplayEffectSpecToSelf(
				*specHandle.Data.Get());

			if(activeGEHandle.WasSuccessfullyApplied() == false)
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to apply jump effect for %s !"), *GetNameSafe(Character));
			}
		}
		
			
	}
}