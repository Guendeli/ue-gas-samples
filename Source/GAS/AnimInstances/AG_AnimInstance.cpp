// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimInstances/AG_AnimInstance.h"
#include "GASCharacter.h"
#include "GASGameTypes.h"
#include "Animation/AnimSequenceBase.h"
#include "Animation/BlendSpace.h"
#include "GAS/DataAssets/CharacterDataAsset.h"
#include "GAS/DataAssets/CharacterAnimDataAsset.h"


UBlendSpace* UAG_AnimInstance::GetLocomotionBlendSpace() const
{
	AGASCharacter* possesedActor = Cast<AGASCharacter>(GetOwningActor());
	if(possesedActor == nullptr)
		return nullptr;

	FCharacterData data = possesedActor->GetCharacterData();
	UCharacterAnimDataAsset* animDataAsset = data.CharacterAnimDataAsset;

	if(animDataAsset)
	{
		return animDataAsset->CharacterAnimData.MovementBlendSpace;
	}

	return DefaultCharacterAnimDataAsset ? DefaultCharacterAnimDataAsset->CharacterAnimData.MovementBlendSpace : nullptr;
}

UAnimSequenceBase* UAG_AnimInstance::GetIdleAnimation() const
{
	AGASCharacter* possesedActor = Cast<AGASCharacter>(GetOwningActor());
	if(possesedActor == nullptr)
		return nullptr;

	FCharacterData data = possesedActor->GetCharacterData();
	UCharacterAnimDataAsset* animDataAsset = data.CharacterAnimDataAsset;

	if(animDataAsset)
	{
		return animDataAsset->CharacterAnimData.IdleAnimationAsset;
	}

	return DefaultCharacterAnimDataAsset ? DefaultCharacterAnimDataAsset->CharacterAnimData.IdleAnimationAsset : nullptr;
}
