// Copyright Epic Games, Inc. All Rights Reserved.

#include "Expeditionist_ReduxGameMode.h"
#include "Expeditionist_ReduxCharacter.h"
#include "UObject/ConstructorHelpers.h"

AExpeditionist_ReduxGameMode::AExpeditionist_ReduxGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
