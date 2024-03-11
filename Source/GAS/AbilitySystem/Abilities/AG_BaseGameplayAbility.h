// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AG_BaseGameplayAbility.generated.h"

class AGASCharacter;
/**
 * 
 */
UCLASS()
class GAS_API UAG_BaseGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:

	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<UGameplayEffect>> OngoingEffectsToRemoveOnEnd;

	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<UGameplayEffect>> OngoingEffectsToApplyOnStart;

	TArray<FActiveGameplayEffectHandle> RemoveOnEndEffectsHandle;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AGASCharacter* GetCharacterFromActorInfo();

};
