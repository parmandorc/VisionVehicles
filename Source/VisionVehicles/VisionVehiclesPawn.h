// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "WheeledVehicle.h"
#include "VisionVehiclesPawn.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UTextRenderComponent;
class UInputComponent;
class UVehicleVisionComponent;
class UNeuralNetwork;
UCLASS(config=Game)
class AVisionVehiclesPawn : public AWheeledVehicle
{
	GENERATED_BODY()

	/** Spring arm that will offset the camera */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArm;

	/** Camera component that will be our viewpoint */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

	/** SCene component for the In-Car view origin */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* InternalCameraBase;

	/** Camera component for the In-Car view */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* InternalCamera;

	/** Text component for the In-Car speed */
	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UTextRenderComponent* InCarSpeed;

	/** Text component for the In-Car gear */
	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UTextRenderComponent* InCarGear;

	/* Vision compoment used by the vehicle */
	UPROPERTY(Category = Vision, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UVehicleVisionComponent* VisionComponent;

	/** The number of inputs of the NN */
	UPROPERTY(Category = "AI|Neural Network", EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", UIMin = "1"))
	int NumberOfInputs;

	/** The number of outputs of the NN */
	UPROPERTY(Category = "AI|Neural Network", EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", UIMin = "1"))
	int NumberOfOutputs;

	/** The dimensions of the hidden layers of the NN */
	UPROPERTY(Category = "AI|Neural Network", EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<int> HiddenLayers;

	/** The initial learning rate of the NN */
	UPROPERTY(Category = "AI|Neural Network", EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float InitialLearningRate;

	/** The decay factor for the learning rate of the NN */
	UPROPERTY(Category = "AI|Neural Network", EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float LearningRateDecay;

public:
	AVisionVehiclesPawn();

	/** The current speed as a string eg 10 km/h */
	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly)
	FText SpeedDisplayString;

	/** The current gear as a string (R,N, 1,2 etc) */
	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly)
	FText GearDisplayString;

	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly)
	/** The color of the incar gear text in forward gears */
	FColor	GearDisplayColor;

	/** The color of the incar gear text when in reverse */
	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly)
	FColor	GearDisplayReverseColor;

	/** Are we using incar camera */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly)
	bool bInCarCameraActive;

	/** Are we in reverse gear */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly)
	bool bInReverseGear;

	/** Initial offset of incar camera */
	FVector InternalCameraOrigin;
	// Begin Pawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End Pawn interface

	// Begin Actor interface
	virtual void Tick(float Delta) override;
protected:
	virtual void BeginPlay() override;

public:
	// End Actor interface

	/** Handle pressing forwards */
	void MoveForward(float Val);
	/** Setup the strings used on the hud */
	void SetupInCarHUD();

	/** Update the physics material used by the vehicle mesh */
	void UpdatePhysicsMaterial();
	/** Handle pressing right */
	void MoveRight(float Val);
	/** Handle handbrake pressed */
	void OnHandbrakePressed();
	/** Handle handbrake released */
	void OnHandbrakeReleased();
	/** Switch between cameras */
	void OnToggleCamera();
	/** Handle reset VR device */
	void OnResetVR();

	static const FName LookUpBinding;
	static const FName LookRightBinding;

	/* Process the feed from the vision component an creates the inputs array for the NN */
	UFUNCTION(BlueprintCallable)
	TArray<float> ProcessCameraFeed();

	UFUNCTION(BlueprintCallable)
	FVector2D FindTrackEnd(TArray<bool> cameraFeed);

private:
	/** 
	 * Activate In-Car camera. Enable camera and sets visibility of incar hud display
	 *
	 * @param	bState true will enable in car view and set visibility of various if its doesnt match new state
	 * @param	bForce true will force to always change state
	 */
	void EnableIncarView( const bool bState, const bool bForce = false );

	/** Update the gear and speed strings */
	void UpdateHUDStrings();

	/* Are we on a 'slippery' surface */
	bool bIsLowFriction;

	// A reference to the neural network used by this pawn
	UPROPERTY()
	UNeuralNetwork* NeuralNetwork;

public:
	/** Returns SpringArm subobject **/
	FORCEINLINE USpringArmComponent* GetSpringArm() const { return SpringArm; }
	/** Returns Camera subobject **/
	FORCEINLINE UCameraComponent* GetCamera() const { return Camera; }
	/** Returns InternalCamera subobject **/
	FORCEINLINE UCameraComponent* GetInternalCamera() const { return InternalCamera; }
	/** Returns InCarSpeed subobject **/
	FORCEINLINE UTextRenderComponent* GetInCarSpeed() const { return InCarSpeed; }
	/** Returns InCarGear subobject **/
	FORCEINLINE UTextRenderComponent* GetInCarGear() const { return InCarGear; }
	/** Returns vision component **/
	FORCEINLINE UVehicleVisionComponent* GetVisionComponent() const { return VisionComponent; }
	/** Return the neural network used by this pawn */
	UFUNCTION(BlueprintCallable)
	FORCEINLINE UNeuralNetwork* GetNeuralNetwork() const { return NeuralNetwork; }
};
