// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ActorComponents/AG_CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"

static TAutoConsoleVariable<int32> CVarShowTraversal(
	TEXT("ShowDebugTraversal"),
	0,
	TEXT("Draw debug info about traversal")
	TEXT(" 0: off\n")
	TEXT(" 1: on\n"),
	ECVF_Cheat
);

bool UAG_CharacterMovementComponent::TryTraversal(UAbilitySystemComponent* ASC)
{
	for(TSubclassOf<UGameplayAbility> abilityClass : TraversalAbilitiesOrdered)
	{
		if(ASC->TryActivateAbilityByClass(abilityClass, true))
		{
			FGameplayAbilitySpec* spec;
			spec = ASC->FindAbilitySpecFromClass(abilityClass);
			if(spec && spec->IsActive())
			{
				return true;
			}
		}
	}	
	return false;
}
