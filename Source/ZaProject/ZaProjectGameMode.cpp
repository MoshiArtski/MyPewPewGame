// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZaProjectGameMode.h"
#include "ZaProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"

AZaProjectGameMode::AZaProjectGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
