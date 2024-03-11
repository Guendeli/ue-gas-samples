// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ActorComponents/AG_FootstepsComponent.h"
#include "PhysicsMaterials/AG_PhysicalMaterial.h"
#include "Kismet/GameplayStatics.h"
#include "GASCharacter.h"
#include "DrawDebugHelpers.h"

static TAutoConsoleVariable<int32> CVarShowFootsteps(
	TEXT("ShowDebugFootsteps"),
	0,
	TEXT("Shows info about footsteps")
	TEXT(" 0: off\n")
	TEXT(" 1: on\n"),
	ECVF_Cheat
);


// Sets default values for this component's properties
UAG_FootstepsComponent::UAG_FootstepsComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UAG_FootstepsComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UAG_FootstepsComponent::HandleFootstep(EFoot foot)
{
	AGASCharacter* character = Cast<AGASCharacter>(GetOwner());
	if(character != nullptr)
	{
		const int32 isDebug = CVarShowFootsteps.GetValueOnAnyThread();
		
		USkeletalMeshComponent* mesh = character->GetMesh();
		if(mesh->IsValidLowLevel() == false)
			return;

		FHitResult hitResult;
		const FVector socketLocation = mesh->GetSocketLocation(foot == EFoot::Left ? LeftFootSocketName : RightFootSocketName);
		const FVector location = socketLocation + FVector::UpVector * 20; // TODO - Remove Magic numbers

		FCollisionQueryParams queryParams;
		queryParams.bReturnPhysicalMaterial = true;
		queryParams.AddIgnoredActor(character);

		bool trace = GetWorld()->LineTraceSingleByChannel(hitResult, location, location + FVector::UpVector * -50.0f, ECC_WorldStatic, queryParams);
		if(trace)
		{
			if(hitResult.bBlockingHit)
			{
				if(hitResult.PhysMaterial.Get())
				{
					UAG_PhysicalMaterial* physicsMaterial = Cast<UAG_PhysicalMaterial>(hitResult.PhysMaterial.Get());
					if(physicsMaterial != nullptr)
					{
						UGameplayStatics::PlaySoundAtLocation(this, physicsMaterial->FootstepSound, location);
						
						if(isDebug > 0)
						{
							DrawDebugString(GetWorld(), location, physicsMaterial->GetName(), nullptr, FColor::White, 5);
						}
					}

					if(isDebug)
					{
						DrawDebugSphere(GetWorld(), location, 16, 16, FColor::Red, false,4,0,1);
					}
				}
			} else
			{
				DrawDebugLine(GetWorld(), location, location + FVector::UpVector * -50.f, FColor::Red, false, 4, 0,1);
			}
		}
	}
}


