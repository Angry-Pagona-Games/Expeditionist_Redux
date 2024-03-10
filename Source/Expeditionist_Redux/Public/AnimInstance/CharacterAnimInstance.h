// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CharacterAnimInstance.generated.h"


class AExpeditionist_ReduxCharacter;
class UCustomMovementComponent;
/**
 * 
 */
UCLASS()
class EXPEDITIONIST_REDUX_API UCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
	public:
	// the below functions are the native overrides for each phase
	// Native initialization override point
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	private:
		UPROPERTY()
		AExpeditionist_ReduxCharacter* ExpeditionistCharacter;
		
		UPROPERTY()
		UCustomMovementComponent* CustomMovementComponent;
		
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Reference, meta = (AllowPrivateAccess = "true"))
		float GroundSpeed;
		void GetGroundSpeed();
		
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Reference, meta = (AllowPrivateAccess = "true"))
		float AirSpeed;
		void GetAirSpeed();

		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Reference, meta = (AllowPrivateAccess = "true"))
		bool bShouldMove;
		void GetShouldMove();

		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Reference, meta = (AllowPrivateAccess = "true"))
		bool bIsFalling;
		void GetIsFalling();
};