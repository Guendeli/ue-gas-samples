// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AG_BaseGameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AG_VaultOverAbility.generated.h"

UCLASS()
class GAS_API UAG_VaultOverAbility : public UAG_BaseGameplayAbility
{
	GENERATED_BODY()

public:

	UAG_VaultOverAbility();

	virtual bool CommitCheck(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:

	// Horizontal Traces
	UPROPERTY(EditDefaultsOnly, Category="Horitonzal Trace")
	float HorizontalTraceRadius = 30.0f;
	
	UPROPERTY(EditDefaultsOnly, Category="Horitonzal Trace")
	float HorizontalTraceLenght = 500.0f;

	UPROPERTY(EditDefaultsOnly, Category="Horitonzal Trace")
	float HorizontalTraceCount = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category="Horitonzal Trace")
	float HorizontalTraceStep = 30.0f;

	// Vertical Traces
	UPROPERTY(EditDefaultsOnly, Category="Vertical Trace")
	float VerticalTraceRadius = 30.0f;
	
	UPROPERTY(EditDefaultsOnly, Category="Vertical Trace")
	float VerticalTraceStep = 30.0f;

	// Collision Query
	UPROPERTY(EditDefaultsOnly)
	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;

	UPROPERTY(EditDefaultsOnly)
	TArray<TEnumAsByte<ECollisionChannel>> CollisionChannelsToIgnore;

	// Animation
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* VaultMontage = nullptr;

	UPROPERTY()
	UAbilityTask_PlayMontageAndWait* MontageTask = nullptr;
	
	FVector JumpToLocation;
	FVector JumpOverLocation;
};
