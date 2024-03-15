// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AG_BaseGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "GASCharacter.h"

void UAG_BaseGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                              const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                              const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilitySystemComponent* abilitySystem = ActorInfo->AbilitySystemComponent.Get();
	if(abilitySystem == nullptr)
		return;
	
	FGameplayEffectContextHandle effectsContext = abilitySystem->MakeEffectContext();
	for(TSubclassOf<UGameplayEffect> gameplayEffect : OngoingEffectsToApplyOnStart)
	{
		if(!gameplayEffect.Get()) continue;

		FGameplayEffectSpecHandle specHandle = abilitySystem->MakeOutgoingSpec(gameplayEffect, 1, effectsContext);
		if(specHandle.IsValid())
		{
			FActiveGameplayEffectHandle activeGEHandle = abilitySystem->ApplyGameplayEffectSpecToSelf(*specHandle.Data.Get());
		}
	}

	if(IsInstantiated())
	{
		for(TSubclassOf<UGameplayEffect> gameplayEffect : OngoingEffectsToRemoveOnEnd)
		{
			if(!gameplayEffect.Get()) continue;

			FGameplayEffectSpecHandle specHandle = abilitySystem->MakeOutgoingSpec(gameplayEffect, 1, effectsContext);
			if(specHandle.IsValid())
			{
				FActiveGameplayEffectHandle activeGEHandle = abilitySystem->ApplyGameplayEffectSpecToSelf(*specHandle.Data.Get());
				if(activeGEHandle.WasSuccessfullyApplied())
				{
					RemoveOnEndEffectsHandle.Add(activeGEHandle);
				}
			}
		}
	}
}

void UAG_BaseGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	if(IsInstantiated())
	{
		UAbilitySystemComponent* abilitySystem = ActorInfo->AbilitySystemComponent.Get();
		if(abilitySystem == nullptr)
			return;
		
		for(FActiveGameplayEffectHandle activeEffectHandle : RemoveOnEndEffectsHandle)
		{
			abilitySystem->RemoveActiveGameplayEffect(activeEffectHandle);
		}

		RemoveOnEndEffectsHandle.Empty();
	}
	
}

AGASCharacter* UAG_BaseGameplayAbility::GetCharacterFromActorInfo() const
{
	AActor* actor = GetAvatarActorFromActorInfo();
	if(actor)
	{
		return CastChecked<AGASCharacter>(actor);
	}

	return nullptr;
}
