// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/HUD.h"
#include "VisionVehiclesHud.generated.h"


UCLASS(config = Game)
class AVisionVehiclesHud : public AHUD
{
	GENERATED_BODY()

	/** Font used to render the vehicle info */
	UPROPERTY()
	UFont* HUDFont;

	/* Material used to render the vehicle vision info */
	UPROPERTY(Category = Vision, EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UMaterialInterface* VisionMaterial;

	/* Material used to render the vehicle vision info */
	UPROPERTY(Category = Vision, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FVector2D VisionQuadPosition;

	/* Material used to render the vehicle vision info */
	UPROPERTY(Category = Vision, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FVector2D VisionQuadScale;

public:
	AVisionVehiclesHud();

	// Begin AHUD interface
	virtual void DrawHUD() override;
	// End AHUD interface

	// Begin AActor interface
	virtual void BeginPlay() override;
	// End AActor interface

private:
	/* The dynamic material instance used to render the vehicle vision info */
	UPROPERTY()
	UMaterialInstanceDynamic* dynamicVisionMaterial;

	/* The dynamic texture used by the dynamic vision material */
	UTexture2D* dynamicVisionTexture;

	/* The colors array used by the dynamic vision texture */
	uint8* dynamicVisionColors;

	/* The update region of the dynamic vision texture. Needed for the UpdateTextureRegions function */
	FUpdateTextureRegion2D* updateTextureRegion;

	/* Updates the texture used to render the vehicle vision info */
	void UpdateVisionTexture();
};
