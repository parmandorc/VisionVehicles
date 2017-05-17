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

	/* Returns the camera feed as classified data.*/

	TBitArray<FDefaultBitArrayAllocator> GetFeed();

     UFUNCTION(BlueprintCallable)
          TArray<bool> GetCameraFeed();

private:
	/* Converts a color to normalized RGB format. 
	 * This removes the luminosity of a color, making it the same regardless of shadows or bright spots. */
	FLinearColor ToNormalizedRGB(const FLinearColor color) const;
};
