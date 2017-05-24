// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "VisionVehicles.h"
#include "VisionVehiclesPawn.h"
#include "VisionVehiclesWheelFront.h"
#include "VisionVehiclesWheelRear.h"
#include "VisionVehiclesHud.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "WheeledVehicleMovementComponent4W.h"
#include "Engine/SkeletalMesh.h"
#include "Engine.h"
#include "VehicleVisionComponent.h"
#include "NeuralNetwork.h"

// Needed for VR Headset
#if HMD_MODULE_INCLUDED
#include "IHeadMountedDisplay.h"
#endif // HMD_MODULE_INCLUDED

const FName AVisionVehiclesPawn::LookUpBinding("LookUp");
const FName AVisionVehiclesPawn::LookRightBinding("LookRight");

#define LOCTEXT_NAMESPACE "VehiclePawn"

AVisionVehiclesPawn::AVisionVehiclesPawn()
{
	// Car mesh
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> CarMesh(TEXT("/Game/VehicleModel/Sedan/Sedan_SkelMesh.Sedan_SkelMesh"));
	GetMesh()->SetSkeletalMesh(CarMesh.Object);

	static ConstructorHelpers::FClassFinder<UObject> AnimBPClass(TEXT("/Game/VehicleModel/Sedan/Sedan_AnimBP"));
	GetMesh()->SetAnimInstanceClass(AnimBPClass.Class);

	// Simulation
	UWheeledVehicleMovementComponent4W* Vehicle4W = CastChecked<UWheeledVehicleMovementComponent4W>(GetVehicleMovement());

	check(Vehicle4W->WheelSetups.Num() == 4);

	Vehicle4W->WheelSetups[0].WheelClass = UVisionVehiclesWheelFront::StaticClass();
	Vehicle4W->WheelSetups[0].BoneName = FName("Wheel_Front_Left");
	Vehicle4W->WheelSetups[0].AdditionalOffset = FVector(0.f, -12.f, 0.f);

	Vehicle4W->WheelSetups[1].WheelClass = UVisionVehiclesWheelFront::StaticClass();
	Vehicle4W->WheelSetups[1].BoneName = FName("Wheel_Front_Right");
	Vehicle4W->WheelSetups[1].AdditionalOffset = FVector(0.f, 12.f, 0.f);

	Vehicle4W->WheelSetups[2].WheelClass = UVisionVehiclesWheelRear::StaticClass();
	Vehicle4W->WheelSetups[2].BoneName = FName("Wheel_Rear_Left");
	Vehicle4W->WheelSetups[2].AdditionalOffset = FVector(0.f, -12.f, 0.f);

	Vehicle4W->WheelSetups[3].WheelClass = UVisionVehiclesWheelRear::StaticClass();
	Vehicle4W->WheelSetups[3].BoneName = FName("Wheel_Rear_Right");
	Vehicle4W->WheelSetups[3].AdditionalOffset = FVector(0.f, 12.f, 0.f);

	// Create a spring arm component
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->TargetOffset = FVector(0.f, 0.f, 200.f);
	SpringArm->SetRelativeRotation(FRotator(-15.f, 0.f, 0.f));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 600.0f;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 7.f;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;

	// Create camera component 
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	Camera->FieldOfView = 90.f;

	// Create vision component
	VisionComponent = CreateDefaultSubobject<UVehicleVisionComponent>(TEXT("VisionComponent0"));
	VisionComponent->SetRelativeLocation(FVector(150.0f, 0.0f, 180.0f));
	VisionComponent->SetRelativeRotation(FRotator(-30.0f, 0.0f, 0.0f));
	VisionComponent->SetupAttachment(RootComponent);

	// Create In-Car camera component 
	InternalCameraOrigin = FVector(0.0f, -40.0f, 120.0f);

	InternalCameraBase = CreateDefaultSubobject<USceneComponent>(TEXT("InternalCameraBase"));
	InternalCameraBase->SetRelativeLocation(InternalCameraOrigin);
	InternalCameraBase->SetupAttachment(GetMesh());

	InternalCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("InternalCamera"));
	InternalCamera->bUsePawnControlRotation = false;
	InternalCamera->FieldOfView = 90.f;
	InternalCamera->SetupAttachment(InternalCameraBase);

	//Setup TextRenderMaterial
	static ConstructorHelpers::FObjectFinder<UMaterial> TextMaterial(TEXT("Material'/Engine/EngineMaterials/AntiAliasedTextMaterialTranslucent.AntiAliasedTextMaterialTranslucent'"));
	
	UMaterialInterface* Material = TextMaterial.Object;

	// Create text render component for in car speed display
	InCarSpeed = CreateDefaultSubobject<UTextRenderComponent>(TEXT("IncarSpeed"));
	InCarSpeed->SetTextMaterial(Material);
	InCarSpeed->SetRelativeLocation(FVector(70.0f, -75.0f, 99.0f));
	InCarSpeed->SetRelativeRotation(FRotator(18.0f, 180.0f, 0.0f));
	InCarSpeed->SetupAttachment(GetMesh());
	InCarSpeed->SetRelativeScale3D(FVector(1.0f, 0.4f, 0.4f));

	// Create text render component for in car gear display
	InCarGear = CreateDefaultSubobject<UTextRenderComponent>(TEXT("IncarGear"));
	InCarGear->SetTextMaterial(Material);
	InCarGear->SetRelativeLocation(FVector(66.0f, -9.0f, 95.0f));	
	InCarGear->SetRelativeRotation(FRotator(25.0f, 180.0f,0.0f));
	InCarGear->SetRelativeScale3D(FVector(1.0f, 0.4f, 0.4f));
	InCarGear->SetupAttachment(GetMesh());
	
	// Colors for the incar gear display. One for normal one for reverse
	GearDisplayReverseColor = FColor(255, 0, 0, 255);
	GearDisplayColor = FColor(255, 255, 255, 255);

	// Colors for the in-car gear display. One for normal one for reverse
	GearDisplayReverseColor = FColor(255, 0, 0, 255);
	GearDisplayColor = FColor(255, 255, 255, 255);

	bInReverseGear = false;

	// Set NN defaults
	NumberOfInputs = 1;
	NumberOfOutputs = 1;
	InitialLearningRate = 0.1f;
	LearningRateDecay = 0.001f;
}

void AVisionVehiclesPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	//Super::SetupPlayerInputComponent(PlayerInputComponent);

	//// set up gameplay key bindings
	//check(PlayerInputComponent);

	//PlayerInputComponent->BindAxis("MoveForward", this, &AVisionVehiclesPawn::MoveForward);
	//PlayerInputComponent->BindAxis("MoveRight", this, &AVisionVehiclesPawn::MoveRight);
	//PlayerInputComponent->BindAxis("LookUp");
	//PlayerInputComponent->BindAxis("LookRight");

	//PlayerInputComponent->BindAction("Handbrake", IE_Pressed, this, &AVisionVehiclesPawn::OnHandbrakePressed);
	//PlayerInputComponent->BindAction("Handbrake", IE_Released, this, &AVisionVehiclesPawn::OnHandbrakeReleased);
	//PlayerInputComponent->BindAction("SwitchCamera", IE_Pressed, this, &AVisionVehiclesPawn::OnToggleCamera);

	//PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AVisionVehiclesPawn::OnResetVR); 
}

void AVisionVehiclesPawn::MoveForward(float Val)
{
	GetVehicleMovementComponent()->SetThrottleInput(Val);
}

void AVisionVehiclesPawn::MoveRight(float Val)
{
	GetVehicleMovementComponent()->SetSteeringInput(Val);
}

void AVisionVehiclesPawn::OnHandbrakePressed()
{
	GetVehicleMovementComponent()->SetHandbrakeInput(true);
}

void AVisionVehiclesPawn::OnHandbrakeReleased()
{
	GetVehicleMovementComponent()->SetHandbrakeInput(false);
}

void AVisionVehiclesPawn::OnToggleCamera()
{
	EnableIncarView(!bInCarCameraActive);
}

void AVisionVehiclesPawn::EnableIncarView(const bool bState, const bool bForce)
{
	if ((bState != bInCarCameraActive) || ( bForce == true ))
	{
		bInCarCameraActive = bState;
		
		if (bState == true)
		{
			OnResetVR();
			Camera->Deactivate();
			InternalCamera->Activate();
		}
		else
		{
			InternalCamera->Deactivate();
			Camera->Activate();
		}
		
		InCarSpeed->SetVisibility(bInCarCameraActive);
		InCarGear->SetVisibility(bInCarCameraActive);
	}
}


void AVisionVehiclesPawn::Tick(float Delta)
{
	Super::Tick(Delta);

	// Setup the flag to say we are in reverse gear
	bInReverseGear = GetVehicleMovement()->GetCurrentGear() < 0;
	
	// Update the strings used in the hud (incar and onscreen)
	UpdateHUDStrings();

	// Set the string in the incar hud
	SetupInCarHUD();

	bool bHMDActive = false;
#if HMD_MODULE_INCLUDED
	if ((GEngine->HMDDevice.IsValid() == true) && ((GEngine->HMDDevice->IsHeadTrackingAllowed() == true) || (GEngine->IsStereoscopic3D() == true)))
	{
		bHMDActive = true;
	}
#endif // HMD_MODULE_INCLUDED
	if (bHMDActive == false)
	{
		if ( (InputComponent) && (bInCarCameraActive == true ))
		{
			FRotator HeadRotation = InternalCamera->RelativeRotation;
			HeadRotation.Pitch += InputComponent->GetAxisValue(LookUpBinding);
			HeadRotation.Yaw += InputComponent->GetAxisValue(LookRightBinding);
			InternalCamera->RelativeRotation = HeadRotation;
		}
	}
}

void AVisionVehiclesPawn::BeginPlay()
{
	Super::BeginPlay();

	bool bEnableInCar = false;
#if HMD_MODULE_INCLUDED
	bEnableInCar = UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled();
#endif // HMD_MODULE_INCLUDED
	EnableIncarView(bEnableInCar,true);

     // Set neural network
     NeuralNetwork = UNeuralNetwork::GetInstance();

	// Initialize neural network
	NeuralNetwork->Init(NumberOfInputs, NumberOfOutputs, HiddenLayers, InitialLearningRate, LearningRateDecay);
}

void AVisionVehiclesPawn::OnResetVR()
{
#if HMD_MODULE_INCLUDED
	if (GEngine->HMDDevice.IsValid())
	{
		GEngine->HMDDevice->ResetOrientationAndPosition();
		InternalCamera->SetRelativeLocation(InternalCameraOrigin);
		GetController()->SetControlRotation(FRotator());
	}
#endif // HMD_MODULE_INCLUDED
}

void AVisionVehiclesPawn::UpdateHUDStrings()
{
	float KPH = FMath::Abs(GetVehicleMovement()->GetForwardSpeed()) * 0.036f;
	int32 KPH_int = FMath::FloorToInt(KPH);

	// Using FText because this is display text that should be localizable
	SpeedDisplayString = FText::Format(LOCTEXT("SpeedFormat", "{0} km/h"), FText::AsNumber(KPH_int));
	
	if (bInReverseGear == true)
	{
		GearDisplayString = FText(LOCTEXT("ReverseGear", "R"));
	}
	else
	{
		int32 Gear = GetVehicleMovement()->GetCurrentGear();
		GearDisplayString = (Gear == 0) ? LOCTEXT("N", "N") : FText::AsNumber(Gear);
	}	
}

void AVisionVehiclesPawn::SetupInCarHUD()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if ((PlayerController != nullptr) && (InCarSpeed != nullptr) && (InCarGear != nullptr) )
	{
		// Setup the text render component strings
		InCarSpeed->SetText(SpeedDisplayString);
		InCarGear->SetText(GearDisplayString);
		
		if (bInReverseGear == false)
		{
			InCarGear->SetTextRenderColor(GearDisplayColor);
		}
		else
		{
			InCarGear->SetTextRenderColor(GearDisplayReverseColor);
		}
	}
}

bool checked_value = false;


FVector2D AVisionVehiclesPawn::FindTrackEnd(TArray<bool> cameraFeed)
{
     //if (cameraFeed.Num() != size_y*size_x)
     //     return FVector2D(-1, -1);
     //int size_y = sqrt(cameraFeed.Num()),
     //     size_x = size_y;
     //start_point_found = false;
     //end_point_found = false;

     ////go along the left border: (0, Y) -> (0, 0)
     //for (int y = size_y - 1; y >= 0; y--)
     //{
     //     if (cameraFeed[y*size_x] == checked_value && !start_point_found)
     //     {
     //          start_point_found = true;
     //          start_point = FVector2D(0.f, y);
     //     }
     //     if (cameraFeed[y*size_x] == !checked_value && !end_point_found && start_point_found)
     //     {
     //          end_point_found = true;
     //          end_point = FVector2D(0.f, y - 1);
     //     }
     //}
     //
     //go along the top border: (0, 0) -> (X, 0)
     //if (!end_point_found)
     //{

     int size = sqrt(cameraFeed.Num()),
          half_size = size / 2,
          left = 0,
          right = 0;
     for (int i = 0; i < cameraFeed.Num(); i++)
     {
          if (cameraFeed[i] == checked_value)
          {
               if (i%half_size % 2 == 0)
                    left++;
               else
                    right++;
          }
     }
     return (FVector2D(left, right));


     //     for (int x = 0; x < size_x; x++)
     //     {
     //          if (cameraFeed[x] == checked_value && !start_point_found)
     //          {
     //               start_point_found = true;
     //               start_point = FVector2D(x, 0.f);
     //          }
     //          if (cameraFeed[x] == !checked_value && !end_point_found && start_point_found)
     //          {
     //               end_point_found = true;
     //               end_point = FVector2D(x - 1, 0.f);
     //          }
     //     }

     //     ////go along the right border: (X, 0) -> (X, Y)
     //     //if (!end_point_found)
     //     //{
     //     //     for (int y = 0; y < size_y; y++)
     //     //     {
     //     //          if (cameraFeed[(y + 1)*size_x - 1] == checked_value && !start_point_found)
     //     //          {
     //     //               start_point_found = true;
     //     //               start_point = FVector2D(size_x - 1, y);
     //     //          }
     //     //          if (cameraFeed[(y + 1)*size_x - 1] == !checked_value && !end_point_found && start_point_found)
     //     //          {
     //     //               end_point_found = true;
     //     //               end_point = FVector2D(size_x - 1, y - 1);
     //     //          }
     //     //     }
     //     //}
     ////}

     //if (end_point_found)
     //{
     //     return FVector2D(
     //          int((start_point.X + end_point.X) / 2),
     //          int((start_point.Y + end_point.Y) / 2));
     //}
     //return FVector2D(-1, -1);
}

TArray<float> AVisionVehiclesPawn::ProcessCameraFeed()
{
	TBitArray<> feed = GetVisionComponent()->GetFeed();

	// Compute the vertical projection histogram of the image
	TArray<float> verticalProjectionHistogram;
	int n = FMath::Sqrt(feed.Num());
	int totalCount = 0;
	for (int j = 0; j < n; j++)
	{
		int columnCount = 0;
		for (int i = 0; i < n; i++)
		{
			if (feed[i * n + j])
			{
				++columnCount;
				++totalCount;
			}
		}
		verticalProjectionHistogram.Add(columnCount);
	}
	for (int i = 0; i < n; i++)
	{
		verticalProjectionHistogram[i] /= totalCount;
	}

	// Compute the inputs from the vertical projection histogram
	float t = 0.0f, m = 0.5f, s = 0.0f, k = 0.0f, sk = 0.0f, u = 0.0f, e = 0.0f;
	if (totalCount > 0)
	{
		m = 0.0f;
		for (int i = 0; i < n; i++)
		{
			m += i * verticalProjectionHistogram[i];
		}
		for (int i = 0; i < n; i++)
		{
			s += FMath::Pow(i - m, 2.0f) * verticalProjectionHistogram[i];
			//k += FMath::Pow(i - m, 4.0f) * verticalProjectionHistogram[i];
			sk += FMath::Pow(i - m, 3.0f) * verticalProjectionHistogram[i];
			//u += FMath::Pow(verticalProjectionHistogram[i], 2.0f);
			//if (verticalProjectionHistogram[i] > 0.0f)
			//	e -= verticalProjectionHistogram[i] * FMath::Log2(verticalProjectionHistogram[i]);
		}

		s = FMath::Sqrt(s);
		//k = k / FMath::Pow(s, 4.0f) - 3.0f;
		sk = sk / FMath::Pow(s, 3.0f);

		t = (float)totalCount / (n * n);
		m = m / n;
		s = s / n;
	}

	return TArray<float>({ t, m, s, sk, GetVehicleMovement()->GetForwardSpeed() / 2500.0f });
}

#undef LOCTEXT_NAMESPACE
