// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CustomMovementComponent.generated.h"
UENUM(BlueprintType)
namespace ECustomMovementMode
{
	enum Type
	{
		//MOVE_None UMETA(DisplayName = "None"),
		MOVE_Climbing UMETA(DisplayName = "Climbing"),
		//MOVE_MAX,
	};
}

/**
 * 
 */
UCLASS()
class EXPEDITIONIST_REDUX_API UCustomMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
protected:
#pragma region Override Functions

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	virtual float GetMaxSpeed() const override;
	virtual float GetMaxAcceleration() const override;

#pragma endregion

private:
#pragma region ClimbTraces

	TArray<FHitResult> DoCapsuleTraceMultiByObject(const FVector& Start, const FVector& End, bool bShowDebugShape = false, bool DrawPersistantShapes = false);
	FHitResult DoLineTraceSingleByObject(const FVector& Start, const FVector& End, bool bShowDebugShape = false, bool DrawPersistantShapes = false);
#pragma endregion


#pragma region ClimbCore

	bool TraceClimbableSurfaces();
	FHitResult TraceFromEyeHeight(float TraceDistance, float TraceStartOffset = 0.f);
	bool bCanStartClimbing();
	void StartClimbing();
	void StopClimbing();
	void PhysClimbing(float deltaTime, int32 Iterations);
	void ProcessClimbableSurfaceInfo();
	FQuat GetClimbingRotation(float DeltaTime);
	void SnapMovementToClimbableSurface(float DeltaTime);
#pragma endregion

#pragma region ClimbCoreVariables

	TArray<FHitResult> ClimbableSurfacesTracedResults;
	FVector CurrentClimbableSurfaceLocation;
	FVector CurrentClimbableSurfaceNormal;


#pragma endregion

#pragma region ClimbBPVariables

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	TArray<TEnumAsByte<EObjectTypeQuery>> ClimbSurfaceTraceTypes;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta= (AllowPrivateAccess = "true"))
	float ClimbCapsuleTraceRadius = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float ClimbCapsuleTraceHalfHeight = 72.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float MaxBreakClimbDeceleration= 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float MaxClimbSpeed = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float MaxClimbAcceleration = 300.f;
		
#pragma endregion

public:
	void ToggleClimbing(bool bEnableClimbing);	
	bool IsClimbing() const;
	FORCEINLINE FVector GetCurrentClimbableSurfaceNormal() const { return CurrentClimbableSurfaceNormal; }


};
