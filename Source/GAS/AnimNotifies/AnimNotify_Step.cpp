// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifies/AnimNotify_Step.h"
#include "GASGameTypes.h"
#include "AbilitySystem/ActorComponents/AG_FootstepsComponent.h"
#include "GASCharacter.h"

void UAnimNotify_Step::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	check(MeshComp);

	AGASCharacter* character = MeshComp ? Cast<AGASCharacter>(MeshComp->GetOwner()) : nullptr;

	if(character)
	{
		UAG_FootstepsComponent* footstepComp = character->GetFootstepsComponent();
		if(footstepComp)
		{
			footstepComp->HandleFootstep(Foot);
		}
	}
}
