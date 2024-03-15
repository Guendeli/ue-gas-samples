// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AG_Task_TickWallRun.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWallRunWallSideDetermenedDelegate, bool, bLeftSide);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWallRunFinishedDelegate);

class Acharacter;
class UCharacterMovementComponent;

UCLASS()
class GAS_API UAG_Task_TickWallRun : public UAbilityTask
{
	GENERATED_BODY()
public:

	public:
    
    	UPROPERTY(BlueprintAssignable)
    	FOnWallRunFinishedDelegate OnFinished;
    
    	UPROPERTY(BlueprintAssignable)
    	FOnWallRunWallSideDetermenedDelegate OnWallSideDetermened;
    
    	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HiddenPin = "OwningAbility", DefaultToSelf = "OwningAbility"))
    	static UAG_Task_TickWallRun* CreateWallRunTask(UGameplayAbility* OwningAbility, ACharacter* InCharacterOwner, UCharacterMovementComponent* InCharacterMovement, TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes);
    
    	virtual void Activate() override;
    
    	virtual void OnDestroy(bool bInOwnerFinished) override;
    
    	virtual void TickTask(float DeltaTime) override;
    
    protected:
    
    	UCharacterMovementComponent* CharacterMovement = nullptr;
    
    	ACharacter* CharacterOwner = nullptr;
    
    	TArray<TEnumAsByte<EObjectTypeQuery>> WallRun_TraceObjectTypes;
    
    	bool FindRunnableWall(FHitResult& OnWallHit);
    
    	bool IsWallOnTheLeft(const FHitResult& InWallHit) const;

	
};
