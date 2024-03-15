// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Tasks/AG_Task_TickWallRun.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"

void UAG_Task_TickWallRun::Activate()
{
	Super::Activate();

	FHitResult OnWallHit;

	//const FVector CurrentAcceleration = CharacterMovement->GetCurrentAcceleration();

	if (!FindRunnableWall(OnWallHit))
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnFinished.Broadcast();
		}

		EndTask();

		return;
	}

	OnWallSideDetermened.Broadcast(IsWallOnTheLeft(OnWallHit));

	CharacterOwner->Landed(OnWallHit);

	CharacterOwner->SetActorLocation(OnWallHit.ImpactPoint + OnWallHit.ImpactNormal * 60.f);

	CharacterMovement->SetMovementMode(MOVE_Flying);

	CharacterMovement->GravityScale = 0.2f;
}

UAG_Task_TickWallRun* UAG_Task_TickWallRun::CreateWallRunTask(UGameplayAbility* OwningAbility, ACharacter* InCharacterOwner, UCharacterMovementComponent* InCharacterMovement, TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes)
{
	UAG_Task_TickWallRun* WallRunTask = NewAbilityTask<UAG_Task_TickWallRun>(OwningAbility);

	WallRunTask->CharacterMovement = InCharacterMovement;
	WallRunTask->CharacterOwner = InCharacterOwner;
	WallRunTask->bTickingTask = true;
	WallRunTask->WallRun_TraceObjectTypes = TraceObjectTypes;

	return WallRunTask;
}

void UAG_Task_TickWallRun::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	FHitResult OnWallHit;

	//const FVector CurrentAcceleration = CharacterMovement->GetCurrentAcceleration();

	if (!FindRunnableWall(OnWallHit))
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnFinished.Broadcast();
		}

		EndTask();

		return;
	}

	const FRotator DirectionRotator = IsWallOnTheLeft(OnWallHit) ? FRotator(0, -90, 0) : FRotator(0, 90, 0);

	const FVector WallRunDirection = DirectionRotator.RotateVector(OnWallHit.ImpactNormal);

	const float CachedZ = CharacterMovement->Velocity.Z;

	CharacterMovement->Velocity = WallRunDirection * 700.f;

	CharacterMovement->Velocity.Z = 0;

	CharacterMovement->SetPlaneConstraintEnabled(true);
	CharacterMovement->SetPlaneConstraintOrigin(OnWallHit.ImpactPoint);
	CharacterMovement->SetPlaneConstraintNormal(OnWallHit.ImpactNormal);
}

bool UAG_Task_TickWallRun::FindRunnableWall(FHitResult& OnWallHit)
{
	const FVector CharacterLocation = CharacterOwner->GetActorLocation();

	const FVector RightVector = CharacterOwner->GetActorRightVector();
	const FVector ForwardVector = CharacterOwner->GetActorForwardVector();

	const float TraceLength = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius() + 30.f;

	TArray<AActor*> ActorsToIgnore = {CharacterOwner};

	FHitResult TraceHit;

	static const auto CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("ShowDebugTraversal"));
	const bool bShowTraversal = CVar->GetInt() > 0;

	EDrawDebugTrace::Type DebugDrawType = bShowTraversal ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

	if (UKismetSystemLibrary::LineTraceSingleForObjects(GetWorld(), CharacterLocation, CharacterLocation + ForwardVector * TraceLength, WallRun_TraceObjectTypes, true, ActorsToIgnore, DebugDrawType, OnWallHit, true))
	{
		return false;
	}

	if (UKismetSystemLibrary::LineTraceSingleForObjects(GetWorld(), CharacterLocation, CharacterLocation + -RightVector * TraceLength, WallRun_TraceObjectTypes, true, ActorsToIgnore, DebugDrawType, OnWallHit, true))
	{
		if (FVector::DotProduct(OnWallHit.ImpactNormal, RightVector) > 0.3f)
		{
			return true;
		}
	}

	if (UKismetSystemLibrary::LineTraceSingleForObjects(GetWorld(), CharacterLocation, CharacterLocation + RightVector * TraceLength, WallRun_TraceObjectTypes, true, ActorsToIgnore, DebugDrawType, OnWallHit, true))
	{
		if (FVector::DotProduct(OnWallHit.ImpactNormal, -RightVector) > 0.3f)
		{
			return true;
		}
	}

	return false;
}

bool UAG_Task_TickWallRun::IsWallOnTheLeft(const FHitResult& InWallHit) const
{
	return FVector::DotProduct(CharacterOwner->GetActorRightVector(), InWallHit.ImpactNormal) > 0.f;
}

void UAG_Task_TickWallRun::OnDestroy(bool bInOwnerFinished)
{
	CharacterMovement->SetPlaneConstraintEnabled(false);

	CharacterMovement->SetMovementMode(MOVE_Falling);

	CharacterMovement->GravityScale = 1.75f;

	Super::OnDestroy(bInOwnerFinished);
}
