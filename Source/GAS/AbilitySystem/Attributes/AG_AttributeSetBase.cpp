// Fill out your copyright notice in the Description page of Project Settings.


#include "AG_AttributeSetBase.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

void UAG_AttributeSetBase::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if(Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	} else if (Data.EvaluatedData.Attribute == GetMovementSpeedAttribute())
	{
		ACharacter* possesedCharacter = Cast<ACharacter>(GetOwningActor());
		UCharacterMovementComponent* characterMovement = possesedCharacter ? possesedCharacter->GetCharacterMovement() : nullptr;
		
		if(characterMovement != nullptr)
		{
			const float speed = GetMovementSpeed();
			characterMovement->MaxWalkSpeed = speed;
		}
	}
}

void UAG_AttributeSetBase::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAG_AttributeSetBase, Health, OldHealth);
}

void UAG_AttributeSetBase::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAG_AttributeSetBase, MaxHealth, OldMaxHealth);
}

void UAG_AttributeSetBase::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAG_AttributeSetBase, Stamina, OldStamina);

}

void UAG_AttributeSetBase::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAG_AttributeSetBase, MaxStamina, OldMaxStamina);
}

void UAG_AttributeSetBase::OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAG_AttributeSetBase, MovementSpeed, OldMovementSpeed);
}

void UAG_AttributeSetBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAG_AttributeSetBase, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAG_AttributeSetBase, MaxHealth, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UAG_AttributeSetBase, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAG_AttributeSetBase, MaxStamina, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UAG_AttributeSetBase, MovementSpeed, COND_None, REPNOTIFY_Always);

}
