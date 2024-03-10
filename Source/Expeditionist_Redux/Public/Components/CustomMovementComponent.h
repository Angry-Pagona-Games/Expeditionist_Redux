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
	
public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
private:
#pragma region ClimbTraces

	TArray<FHitResult> DoCapsuleTraceMultiByObject(const FVector& Start, const FVector& End, bool bShowDebugShape = false, bool DrawPersistantShapes = false);
	FHitResult DoLineTraceSingleByObject(const FVector& Start, const FVector& End, bool bShowDebugShape = false, bool DrawPersistantShapes = false);
#pragma endregion


#pragma region ClimbCore

	bool TraceClimbableSurfaces();
	FHitResult TraceFromEyeHeight(float TraceDistance, float TraceStartOffset = 0.f);
	bool bCanStartClimbing();
#pragma endregion

#pragma region ClimbCoreVariables
	TArray<FHitResult> ClimbableSurfacesTracedResults;

#pragma region ClimbBPVariables

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	TArray<TEnumAsByte<EObjectTypeQuery>> ClimbSurfaceTraceTypes;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta= (AllowPrivateAccess = "true"))
	float ClimbCapsuleTraceRadius = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float ClimbCapsuleTraceHalfHeight = 72.0f;
#pragma endregion

public:
	void ToggleClimbing(bool bEnableClimbing);	
	bool IsClimbing() const;



};
