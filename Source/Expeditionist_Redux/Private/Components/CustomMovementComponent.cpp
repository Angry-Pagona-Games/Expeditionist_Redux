// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CustomMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Expeditionist_Redux/Expeditionist_ReduxCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Expeditionist_Redux/DebugHelper.h"

void UCustomMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	//TODO: Implement climbing logic From Lesson 14
	
	
	//TraceClimbableSurfaces();
	//TraceFromEyeHeight(100.0f);
}
#pragma region Override Functions
void UCustomMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
		Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

		if (IsClimbing())
		{
			Debug::Print(TEXT("Climbing"));
			bOrientRotationToMovement = false;
			CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(48.0f);
		}

		if (PreviousCustomMode == ECustomMovementMode::MOVE_Climbing)
		{
			Debug::Print(TEXT("Stopped Climbing"));
			bOrientRotationToMovement = true;
			CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(96.0f);
			StopMovementImmediately();
		}

		Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
		
}

void UCustomMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	if (IsClimbing())
	{
		PhysClimbing(deltaTime, Iterations);	
	}
	
		Super::PhysCustom(deltaTime, Iterations);
	
}

#pragma endregion
#pragma region ClimbTraces
TArray<FHitResult> UCustomMovementComponent::DoCapsuleTraceMultiByObject(const FVector& Start, const FVector& End, bool bShowDebugShape, bool bDrawPersistantShapes)
{
	TArray<FHitResult> OutCapsuleTraceHitResults;

	EDrawDebugTrace::Type DebugTraceType = EDrawDebugTrace::None;
	if (bShowDebugShape)
	{
		DebugTraceType = EDrawDebugTrace::ForOneFrame;
	}
	if (bDrawPersistantShapes)
	{
		DebugTraceType = EDrawDebugTrace::ForDuration;
	}

	UKismetSystemLibrary::CapsuleTraceMultiForObjects(
		this,
		Start,
		End,
		ClimbCapsuleTraceRadius,
		ClimbCapsuleTraceHalfHeight,
		ClimbSurfaceTraceTypes,
		false,
		TArray<AActor*>(),
		DebugTraceType,
		OutCapsuleTraceHitResults,
		false
	);

	return OutCapsuleTraceHitResults;
}
FHitResult UCustomMovementComponent::DoLineTraceSingleByObject(const FVector& Start, const FVector& End, bool bShowDebugShape, bool bDrawPersistantShapes)
{
	FHitResult OutHit;
	EDrawDebugTrace::Type DebugTraceType = EDrawDebugTrace::None;
	if (bShowDebugShape)
	{
		DebugTraceType = EDrawDebugTrace::ForDuration;
	}
	if (bDrawPersistantShapes)
	{
		DebugTraceType = EDrawDebugTrace::ForDuration;
	}
	
	UKismetSystemLibrary::LineTraceSingleForObjects(
		this,
		Start,
		End,
		ClimbSurfaceTraceTypes,
		false,
		TArray<AActor*>(),
		DebugTraceType,
		OutHit,
		false
	);
	return OutHit;

}
#pragma endregion
#pragma region ClimbCore
void UCustomMovementComponent::ToggleClimbing(bool bEnableClimbing)
{
	if (bEnableClimbing)
	{
		// Enable climbing
		if (bCanStartClimbing())
		{
			// Start climbing
			Debug::Print(TEXT("Can Start Climbing"));
			StartClimbing(ClimbableSurfacesTracedResults[0]);
		}
		else
		{
			// Disable climbing
			Debug::Print(TEXT("Cannot Start Climbing"));
		}
	}
	else
	{
		// Stop climbing
		StopClimbing();
	}
}

bool UCustomMovementComponent::bCanStartClimbing()
{
	// Check if the character is currently falling
	if(IsFalling()) return false;

	//Check for climbable surfaces
	if(!TraceClimbableSurfaces()) return false;

	//Check if the character eyes are looking at a climbable surface
	if(!TraceFromEyeHeight(100.0f).bBlockingHit) return false;

	return true;

}

void UCustomMovementComponent::StartClimbing(const FHitResult& HitResult)
{
		// Set the movement mode to climbing
	SetMovementMode(MOVE_Custom, ECustomMovementMode::MOVE_Climbing);

}

void UCustomMovementComponent::StopClimbing()
{
		// Set the movement mode to walking
	SetMovementMode(MOVE_Walking);
}

void UCustomMovementComponent::PhysClimbing(float deltaTime, int32 Iterations)
{

	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}
	//Process the Climbable Surface Info 
	TraceClimbableSurfaces();
	ProcessClimbableSurfaceInfo();
	/*Check if player should start Climing*/
	RestorePreAdditiveRootMotionVelocity();

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		/*Define max Climb Speed and acceleration*/
		CalcVelocity(deltaTime, 0.f, true, MaxBreakClimbDeceleration);
	}

	ApplyRootMotionToVelocity(deltaTime);

	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	const FVector Adjusted = Velocity * deltaTime;
	FHitResult Hit(1.f);

	/*Handle Climb Rotation*/
	SafeMoveUpdatedComponent(Adjusted, UpdatedComponent->GetComponentQuat(), true, Hit);

	if (Hit.Time < 1.f)
	{
		//adjust and try again
		HandleImpact(Hit, deltaTime, Adjusted);
		SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);

	}

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;
	}
	/*Snap Movement to Climbable Surfaces*/
}

void UCustomMovementComponent::ProcessClimbableSurfaceInfo()
{
	CurrentClimbableSurfaceLocation = FVector::ZeroVector;	
	CurrentClimbableSurfaceNormal = FVector::ZeroVector;

	if (ClimbableSurfacesTracedResults.IsEmpty()) return;
	
//Get the average location and normal of the climbable surfaces
	for (const FHitResult& TracedHitResult: ClimbableSurfacesTracedResults)
	{
		CurrentClimbableSurfaceLocation += TracedHitResult.ImpactPoint;
		CurrentClimbableSurfaceNormal += TracedHitResult.ImpactNormal;
	}
	CurrentClimbableSurfaceLocation /= ClimbableSurfacesTracedResults.Num();
	CurrentClimbableSurfaceNormal = CurrentClimbableSurfaceNormal.GetSafeNormal();

	Debug::Print(TEXT("Climbable Surface Location: %s")+ CurrentClimbableSurfaceLocation.ToString(), FColor::Cyan, 1);
	Debug::Print(TEXT("Climbable Surface Normal: %s") + CurrentClimbableSurfaceNormal.ToString(), FColor::Blue, 1);

}

bool UCustomMovementComponent::IsClimbing() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == ECustomMovementMode::MOVE_Climbing;
}

//Trace for climbable surfaces, if found, return true. Otherwise, return false.
bool UCustomMovementComponent::TraceClimbableSurfaces()
{
	const FVector StartOffset = UpdatedComponent->GetForwardVector() * 30.0f;
	const FVector Start = UpdatedComponent->GetComponentLocation() + StartOffset;
	const FVector End = Start + (UpdatedComponent->GetForwardVector());

	ClimbableSurfacesTracedResults = DoCapsuleTraceMultiByObject(Start, End, true);

	return !ClimbableSurfacesTracedResults.IsEmpty();
}
FHitResult UCustomMovementComponent::TraceFromEyeHeight(float TraceDistance, float TraceStartOffset)
{
	const FVector ComponentLocation = UpdatedComponent->GetComponentLocation();
	const FVector EyeHeightOffset = UpdatedComponent->GetUpVector() * (CharacterOwner->BaseEyeHeight + TraceStartOffset);
	const FVector Start = ComponentLocation + EyeHeightOffset;
	const FVector End = Start + (UpdatedComponent->GetForwardVector()* TraceDistance);

	return DoLineTraceSingleByObject(Start, End, true, true);
}


#pragma endregion