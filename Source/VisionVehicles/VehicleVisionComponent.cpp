// Fill out your copyright notice in the Description page of Project Settings.

#include "VisionVehicles.h"
#include "VehicleVisionComponent.h"

UVehicleVisionComponent::UVehicleVisionComponent()
{
	// Set defaults
	ClassColor = FLinearColor::Red;
	ClassColorDistanceThreshold = 1.25f;

	if (TextureTarget != nullptr)
	{
		//TextureTarget = DuplicateObject(TextureTarget, nullptr);
	}
}

void UVehicleVisionComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (TickType == ELevelTick::LEVELTICK_All)
	{
		TBitArray<FDefaultBitArrayAllocator> cameraFeed = GetFeed();

		// Count positive pixels
		int32 classCount = 0;
		for (int i = 0; i < cameraFeed.Num(); i++)
		{
			if (cameraFeed[i]) ++classCount;
		}

		UE_LOG(LogTemp, Log, TEXT("Positive pixels count: %f"), (float)classCount / cameraFeed.Num());
	}
}

TBitArray<FDefaultBitArrayAllocator> UVehicleVisionComponent::GetFeed()
{
	// Get the raw feed from the camera
	TArray<FColor> rawCameraFeed;
	FTextureRenderTargetResource* renderTarget = TextureTarget->GameThread_GetRenderTargetResource();
	renderTarget->ReadPixels(rawCameraFeed);

	// Transform the raw feed into classified data
	int32 numPixels = rawCameraFeed.Num();
	TBitArray<FDefaultBitArrayAllocator> feed;
	feed.Init(false, numPixels);
	for (int i = 0; i < numPixels; i++)
	{
		if (FLinearColor::Dist(rawCameraFeed[i].ReinterpretAsLinear(), ClassColor) < ClassColorDistanceThreshold)
		{
			feed[i] = true;
		}
	}

	return feed;
}