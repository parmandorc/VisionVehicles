// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/SceneCaptureComponent2D.h"
#include "VehicleVisionComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = Vision, meta = (BlueprintSpawnableComponent))
class VISIONVEHICLES_API UVehicleVisionComponent : public USceneCaptureComponent2D
{
	GENERATED_BODY()

	/* The color to use as class tag. All elements in the camera feed that match this color will be positively classified.*/
	UPROPERTY(Category = Vision, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FLinearColor ClassColor;

	/* The margin of classification, as the euclidean distance between the pixel value and the ClassColor.*/
	UPROPERTY(Category = Vision, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float ClassColorDistanceThreshold;

public:
	UVehicleVisionComponent();

	// BEGIN ACTOR COMPONENT INTERFACE
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	// END ACTOR COMPONENT INTERFACE

	/* Returns the camera feed as classified data.*/
	TBitArray<FDefaultBitArrayAllocator> GetFeed();
};
