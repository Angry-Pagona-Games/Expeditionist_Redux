// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CustomMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Expeditionist_Redux/Expeditionist_ReduxCharacter.h"
#include "Expeditionist_Redux/DebugHelper.h"

void UCustomMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	//TODO: Implement climbing logic From Lesson 14
	
	
	//TraceClimbableSurfaces();
	//TraceFromEyeHeight(100.0f);
}


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

	ClimbableSurfacesTracedResults = DoCapsuleTraceMultiByObject(Start, End, true, true);

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