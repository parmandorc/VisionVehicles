// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "VisionVehicles.h"
#include "VisionVehiclesHud.h"
#include "VisionVehiclesPawn.h"
#include "WheeledVehicle.h"
#include "RenderResource.h"
#include "Shader.h"
#include "Engine/Canvas.h"
#include "WheeledVehicleMovementComponent.h"
#include "Engine/Font.h"
#include "CanvasItem.h"
#include "Engine.h"
#include "VehicleVisionComponent.h"

// Needed for VR Headset
#if HMD_MODULE_INCLUDED
#include "IHeadMountedDisplay.h"
#endif // HMD_MODULE_INCLUDED 

// Function for enqueueing tasks on the render thread.
// This is taken from: https://wiki.unrealengine.com/Dynamic_Textures and https://wiki.unrealengine.com/Procedural_Materials
void UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData)
{
	if (Texture->Resource)
	{
		struct FUpdateTextureRegionsData
		{
			FTexture2DResource* Texture2DResource;
			int32 MipIndex;
			uint32 NumRegions;
			FUpdateTextureRegion2D* Regions;
			uint32 SrcPitch;
			uint32 SrcBpp;
			uint8* SrcData;
		};

		FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;

		RegionData->Texture2DResource = (FTexture2DResource*)Texture->Resource;
		RegionData->MipIndex = MipIndex;
		RegionData->NumRegions = NumRegions;
		RegionData->Regions = Regions;
		RegionData->SrcPitch = SrcPitch;
		RegionData->SrcBpp = SrcBpp;
		RegionData->SrcData = SrcData;

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			UpdateTextureRegionsData,
			FUpdateTextureRegionsData*, RegionData, RegionData,
			bool, bFreeData, bFreeData,
			{
				for (uint32 RegionIndex = 0; RegionIndex < RegionData->NumRegions; ++RegionIndex)
				{
					int32 CurrentFirstMip = RegionData->Texture2DResource->GetCurrentFirstMip();
					if (RegionData->MipIndex >= CurrentFirstMip)
					{
						RHIUpdateTexture2D(
							RegionData->Texture2DResource->GetTexture2DRHI(),
							RegionData->MipIndex - CurrentFirstMip,
							RegionData->Regions[RegionIndex],
							RegionData->SrcPitch,
							RegionData->SrcData
							+ RegionData->Regions[RegionIndex].SrcY * RegionData->SrcPitch
							+ RegionData->Regions[RegionIndex].SrcX * RegionData->SrcBpp
						);
					}
				}
		if (bFreeData)
		{
			FMemory::Free(RegionData->Regions);
			FMemory::Free(RegionData->SrcData);
		}
		delete RegionData;
			});
	}
}


#define LOCTEXT_NAMESPACE "VehicleHUD"

AVisionVehiclesHud::AVisionVehiclesHud()
{
	// Set defaults
	VisionQuadPosition = FVector2D(0.0f, 0.0f);
	VisionQuadScale = FVector2D(200.0f, 200.0f);

	static ConstructorHelpers::FObjectFinder<UFont> Font(TEXT("/Engine/EngineFonts/RobotoDistanceField"));
	HUDFont = Font.Object;
}

void AVisionVehiclesHud::BeginPlay()
{
	if (VisionMaterial != nullptr)
	{
		// Get our vehicle so we can initialize the dynamic texture from the vision feed
		AVisionVehiclesPawn* vehicle = Cast<AVisionVehiclesPawn>(GetOwningPawn());
		UVehicleVisionComponent* visionComponent = nullptr;
		if ((vehicle != nullptr) && ((visionComponent = vehicle->GetVisionComponent()) != nullptr))
		{
			TBitArray<> visionFeed = visionComponent->GetFeed();
			int32 textureSize = FMath::Sqrt(visionFeed.Num());

			// Create the dynamic vision material
			dynamicVisionMaterial = UMaterialInstanceDynamic::Create(VisionMaterial, vehicle);

			// Create a dynamic texture with the default compression (B8G8R8A8)
			dynamicVisionTexture = UTexture2D::CreateTransient(textureSize, textureSize);
			dynamicVisionTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap; //Make sure it won't be compressed
			dynamicVisionTexture->SRGB = 0; //Turn off Gamma-correction
			dynamicVisionTexture->AddToRoot(); //Guarantee no garbage collection by adding it as a root reference
			dynamicVisionTexture->UpdateResource(); //Update the texture with new variable values.

			// Initalize our dynamic pixel array with data size
			dynamicVisionColors = new uint8[textureSize * textureSize * 4]; // * 4 because each color is made out of 4 uint8
			for (int i = 0; i < textureSize * textureSize; i++)
			{
				dynamicVisionColors[i * 4 + 0] = 0;
				dynamicVisionColors[i * 4 + 1] = 0;
				dynamicVisionColors[i * 4 + 2] = 0;
				dynamicVisionColors[i * 4 + 3] = 255;
			}

			// Create a new texture region with the width and height of our dynamic texture
			updateTextureRegion = new FUpdateTextureRegion2D(0, 0, 0, 0, textureSize, textureSize);

			// Update texture and assign it to material
			UpdateTextureRegions(dynamicVisionTexture, 0, 1, updateTextureRegion, (uint32)(textureSize * 4), (uint32)4, dynamicVisionColors, false);
			dynamicVisionMaterial->SetTextureParameterValue("DynamicTextureParam", dynamicVisionTexture);
		}
	}
}

void AVisionVehiclesHud::DrawHUD()
{
	Super::DrawHUD();

	// Calculate ratio from 720p
	const float HUDXRatio = Canvas->SizeX / 1280.f;
	const float HUDYRatio = Canvas->SizeY / 720.f;

	bool bWantHUD = true;
#if HMD_MODULE_INCLUDED
	if (GEngine->HMDDevice.IsValid() == true)
	{
		bWantHUD = GEngine->HMDDevice->IsStereoEnabled();
	}
#endif // HMD_MODULE_INCLUDED
	// We dont want the onscreen hud when using a HMD device	
	if (bWantHUD == true)
	{
		// Get our vehicle so we can check if we are in car. If we are we don't want onscreen HUD
		AVisionVehiclesPawn* Vehicle = Cast<AVisionVehiclesPawn>(GetOwningPawn());
		if ((Vehicle != nullptr) && (Vehicle->bInCarCameraActive == false))
		{
			FVector2D ScaleVec(HUDYRatio * 1.4f, HUDYRatio * 1.4f);

			// Speed
			FCanvasTextItem SpeedTextItem(FVector2D(HUDXRatio * 805.f, HUDYRatio * 455), Vehicle->SpeedDisplayString, HUDFont, FLinearColor::White);
			SpeedTextItem.Scale = ScaleVec;
			Canvas->DrawItem(SpeedTextItem);

			// Gear
			FCanvasTextItem GearTextItem(FVector2D(HUDXRatio * 805.f, HUDYRatio * 500.f), Vehicle->GearDisplayString, HUDFont, Vehicle->bInReverseGear == false ? Vehicle->GearDisplayColor : Vehicle->GearDisplayReverseColor);
			GearTextItem.Scale = ScaleVec;
			Canvas->DrawItem(GearTextItem);
		}

		// Vehicle vision
		UpdateVisionTexture();
		DrawMaterialSimple(dynamicVisionMaterial, VisionQuadPosition.X, VisionQuadPosition.Y, VisionQuadScale.X, VisionQuadScale.Y);
	}
}

void AVisionVehiclesHud::UpdateVisionTexture()
{
	// Get our vehicle so we can initialize the dynamic texture from the vision feed
	AVisionVehiclesPawn* vehicle = Cast<AVisionVehiclesPawn>(GetOwningPawn());
	UVehicleVisionComponent* visionComponent = nullptr;
	if ((vehicle != nullptr) && ((visionComponent = vehicle->GetVisionComponent()) != nullptr))
	{
		TBitArray<> visionFeed = visionComponent->GetFeed();
		int32 textureSize = FMath::Sqrt(visionFeed.Num());

		// Build the vision HUD texture
		for (int i = 0; i < visionFeed.Num(); ++i)
		{
			// Set pixel to white if positive, black otherwise
			bool value = visionFeed[i];
			dynamicVisionColors[i * 4 + 0] = value * 255;
			dynamicVisionColors[i * 4 + 1] = value * 255;
			dynamicVisionColors[i * 4 + 2] = value * 255;
			dynamicVisionColors[i * 4 + 3] = 255;
		}

		// Update texture and assign it to material
		UpdateTextureRegions(dynamicVisionTexture, 0, 1, updateTextureRegion, (uint32)(textureSize * 4), (uint32)4, dynamicVisionColors, false);
		dynamicVisionMaterial->SetTextureParameterValue("DynamicTextureParam", dynamicVisionTexture);
	}
}

#undef LOCTEXT_NAMESPACE
