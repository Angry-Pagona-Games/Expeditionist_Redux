// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimInstance/CharacterAnimInstance.h"
#include "Expeditionist_Redux/Expeditionist_ReduxCharacter.h"
#include "Components/CustomMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"


void UCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	ExpeditionistCharacter = Cast<AExpeditionist_ReduxCharacter>(TryGetPawnOwner());
	if (ExpeditionistCharacter)
	{
		CustomMovementComponent = ExpeditionistCharacter->GetCustomMovementComponent();
	}
}

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!ExpeditionistCharacter || !CustomMovementComponent) return;	
	{
		// Update the animation properties
		GetGroundSpeed();
		GetAirSpeed();
		GetShouldMove();
		GetIsFalling();

	}
}

void UCharacterAnimInstance::GetGroundSpeed()
{
	GroundSpeed = UKismetMathLibrary::VSizeXY(ExpeditionistCharacter->GetVelocity());
}

void UCharacterAnimInstance::GetAirSpeed()
{
	AirSpeed = ExpeditionistCharacter->GetVelocity().Z;
}

void UCharacterAnimInstance::GetShouldMove()
{
	CustomMovementComponent->GetCurrentAcceleration().Size() > 0 &&
		GroundSpeed > 5.f &&
		!bIsFalling;
}

void UCharacterAnimInstance::GetIsFalling()
{
	bIsFalling = CustomMovementComponent->IsFalling();
}

