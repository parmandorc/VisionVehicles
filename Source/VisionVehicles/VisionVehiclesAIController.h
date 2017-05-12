// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIController.h"
#include "VisionVehiclesAIController.generated.h"

/**
 * 
 */
UCLASS()
class VISIONVEHICLES_API AVisionVehiclesAIController : public AAIController
{
     GENERATED_BODY()

     UFUNCTION(BlueprintCallable)
     FVector2D FindTrackEnd(TArray<bool> cameraFeed, int size_y, int size_x);
};
