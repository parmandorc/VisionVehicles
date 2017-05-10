// Fill out your copyright notice in the Description page of Project Settings.

#include "VisionVehicles.h"
#include "VehicleVisionComponent.h"

UVehicleVisionComponent::UVehicleVisionComponent()
{
	// Set defaults
	ClassColor = FLinearColor::Red;
	ClassColorDistanceThreshold = 0.5f;

	if (TextureTarget != nullptr)
	{
		//TextureTarget = DuplicateObject(TextureTarget, nullptr);
	}
}

TBitArray<FDefaultBitArrayAllocator> UVehicleVisionComponent::GetFeed()
{
	// Get the raw feed from the camera
	TArray<FColor> rawCameraFeed;
	FTextureRenderTargetResource* renderTarget = TextureTarget->GameThread_GetRenderTargetResource();
	renderTarget->ReadPixels(rawCameraFeed);

	// Transform the raw feed into classified data
	FLinearColor normalizedClassColor = ToNormalizedRGB(ClassColor);
	int32 numPixels = rawCameraFeed.Num();
	TBitArray<FDefaultBitArrayAllocator> feed;
	feed.Init(false, numPixels);
	for (int i = 0; i < numPixels; i++)
	{
		// Use euclidean distance between normalized colors to determine if it belongs to the class
		if (FLinearColor::Dist(ToNormalizedRGB(rawCameraFeed[i].ReinterpretAsLinear()), normalizedClassColor) < ClassColorDistanceThreshold)
		{
			feed[i] = true;
		}
	}

	return feed;
}

FLinearColor UVehicleVisionComponent::ToNormalizedRGB(const FLinearColor color) const
{
	FVector colorAsVector(color.R, color.G, color.B);
	colorAsVector.Normalize();
	return FLinearColor(colorAsVector.X, colorAsVector.Y, colorAsVector.Z, 1.0f);
}