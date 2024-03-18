// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AG_BaseGameplayAbility.h"
#include "AG_JumpAbility.generated.h"

class UGameplayEffect;

UCLASS()
class GAS_API UAG_JumpAbility : public UAG_BaseGameplayAbility
{
	GENERATED_BODY()

	UAG_JumpAbility();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag WallRunStateTag;

	UPROPERTY(EditDefaultsOnly)
	float OffWallJumpStrenght = 100.0f;
};
