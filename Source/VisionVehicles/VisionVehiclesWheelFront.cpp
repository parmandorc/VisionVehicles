// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "VisionVehicles.h"
#include "VisionVehiclesWheelFront.h"

UVisionVehiclesWheelFront::UVisionVehiclesWheelFront()
{
	ShapeRadius = 35.f;
	ShapeWidth = 10.0f;
	bAffectedByHandbrake = false;
	SteerAngle = 50.f;
}
