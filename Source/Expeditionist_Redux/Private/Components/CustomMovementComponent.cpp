// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/CustomMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Expeditionist_Redux/Expeditionist_ReduxCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Expeditionist_Redux/DebugHelper.h"

void UCustomMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	

	
	//TraceClimbableSurfaces();
	//TraceFromEyeHeight(100.0f);
}
#pragma region Override Functions
void UCustomMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{

	if (IsClimbing())
	{
		bOrientRotationToMovement = false;
		CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(48.f);
	}

	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == ECustomMovementMode::MOVE_Climbing)
	{
		bOrientRotationToMovement = true;
		CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(96.f);

		//Reset the rotation of the character to standing position
		const FRotator DirtyRoation = UpdatedComponent->GetComponentRotation();
		const FRotator CleanStandRotation = FRotator(0.0f, DirtyRoation.Yaw, 0.0f);
		UpdatedComponent->SetRelativeRotation(CleanStandRotation.Quaternion());
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

float UCustomMovementComponent::GetMaxSpeed() const
{
	if (IsClimbing())
	{
		return MaxClimbSpeed;
	}
	else
	{
		return Super::GetMaxSpeed();
	}

}

float UCustomMovementComponent::GetMaxAcceleration() const
{
	if (IsClimbing())
	{
		return MaxClimbAcceleration;
	}
	else
	{
		return Super::GetMaxAcceleration();
	}
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
			
			StartClimbing();
		}
		else
		{
			// Disable climbing
			
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

void UCustomMovementComponent::StartClimbing()
{
	// Set the movement mode to climbing
	SetMovementMode(MOVE_Custom, ECustomMovementMode::MOVE_Climbing);
}

void UCustomMovementComponent::StopClimbing()
{
		// Set the movement mode to falling
	SetMovementMode(MOVE_Falling);
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
	if (CheckShouldStopClimbing())
	{
		StopClimbing();
		
	}

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
	SafeMoveUpdatedComponent(Adjusted, GetClimbingRotation(deltaTime), true, Hit);

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
	SnapMovementToClimbableSurface(deltaTime);

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

}

bool UCustomMovementComponent::CheckShouldStopClimbing()
{
	if (ClimbableSurfacesTracedResults.IsEmpty()) return true;
	
	const float DotResult = FVector::DotProduct(CurrentClimbableSurfaceNormal, FVector::UpVector);
	const float DegreeDiff= FMath::RadiansToDegrees(FMath::Acos(DotResult));

	if (DegreeDiff <= 60.0f)
	{
		return true;
	}
	Debug::Print(TEXT("Degree Diff: ") + FString::SanitizeFloat(DegreeDiff), FColor::Red, 1);

	return false;
}

FQuat UCustomMovementComponent::GetClimbingRotation(float DeltaTime) 
{
	const FQuat CurrentQuat = UpdatedComponent->GetComponentQuat();
	if (HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocity())
	{
		return CurrentQuat;
	}

	//const FQuat TargetQuat = FQuat::Slerp(CurrentQuat, CurrentClimbableSurfaceNormal.ToOrientationQuat(), DeltaTime * 10.0f);
	const FQuat TargetQuat = FRotationMatrix::MakeFromX(-CurrentClimbableSurfaceNormal).ToQuat();
	
	return FMath::QInterpTo(CurrentQuat, TargetQuat, DeltaTime, 5.0f);
	 
}

void UCustomMovementComponent::SnapMovementToClimbableSurface(float DeltaTime)
{
	const FVector ComponentForward = UpdatedComponent->GetForwardVector();
	const FVector ComponentLocation = UpdatedComponent->GetComponentLocation();

	const FVector ProjectedCharacterToSurface =
		(CurrentClimbableSurfaceLocation - ComponentLocation).ProjectOnTo(ComponentForward);

	const FVector SnapVector = -CurrentClimbableSurfaceNormal * ProjectedCharacterToSurface.Length();

	UpdatedComponent->MoveComponent(
		SnapVector * DeltaTime * MaxClimbSpeed,
		UpdatedComponent->GetComponentQuat(),
		true);
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