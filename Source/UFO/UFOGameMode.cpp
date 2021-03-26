// Copyright Epic Games, Inc. All Rights Reserved.

#include "UFOGameMode.h"
#include "UFOPawn.h"

AUFOGameMode::AUFOGameMode()
{
	// set default pawn class to our flying pawn
	DefaultPawnClass = AUFOPawn::StaticClass();
}
