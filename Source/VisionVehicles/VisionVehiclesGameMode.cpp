// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "VisionVehicles.h"
#include "VisionVehiclesGameMode.h"
#include "VisionVehiclesPawn.h"
#include "VisionVehiclesHud.h"

AVisionVehiclesGameMode::AVisionVehiclesGameMode()
{
	DefaultPawnClass = AVisionVehiclesPawn::StaticClass();
	HUDClass = AVisionVehiclesHud::StaticClass();
}
