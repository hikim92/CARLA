// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaUE4.h"
#include "ViaductPlayerController.h"
#include "Vehicle/CarlaWheeledVehicle.h"
#include "Vehicle/ViaductAutopilotComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/KismetMathLibrary.h"



#if PLATFORM_LINUX
	#define USE_G29_PLUGIN
	#include "G29Controls.h"
#endif


// =============================================================================
// -- Constructor and destructor -----------------------------------------------
// =============================================================================

AViaductPlayerController::AViaductPlayerController(const FObjectInitializer& ObjectInitializer)	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	FApp::SetUnfocusedVolumeMultiplier(1.0f);

	bSemiManualControl = false;
	
	// Fetch our ambient Sound Cue
	static ConstructorHelpers::FObjectFinder<USoundCue> AudioCueAsset(TEXT("'/Game/Sounds/Road-Ride_Cue.Road-Ride_Cue'"));
	AudioCue = AudioCueAsset.Object;
	
	AudioComponent = CreateDefaultSubobject<UAudioComponent>("AudioComp");
	AddOwnedComponent(AudioComponent);
	AudioComponent->SetSound(AudioCue);
	AudioComponent->SetAutoActivate(true);
	
	ManualControl.Throttle = 0.0f;
	ManualControl.Steer = 0.0f;
	ManualControl.Brake = 0.0f;
	ManualControl.bHandBrake = false;
	ManualControl.bReverse = false;
	ManualControl.bManualGearShift = false;
	ManualControl.Gear = 0;
}

AViaductPlayerController::~AViaductPlayerController() {}

// =============================================================================
// -- Input --------------------------------------------------------------------
// =============================================================================

void AViaductPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// set up gameplay key bindings
	//InputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	//InputComponent->BindTouch(IE_Released, this, &AVirtualCameraPlayerControllerBase::OnTouchInput);
	//InputComponent->BindAxisKey(EKeys::Gamepad_LeftY, this, &AVirtualCameraPlayerControllerBase::OnMoveForward);
	InputComponent->BindAxis("MoveForward", this, &AViaductPlayerController::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AViaductPlayerController::MoveRight);
	InputComponent->BindAxis("Brake", this, &AViaductPlayerController::Brake);

	InputComponent->BindAction("Handbrake", IE_Pressed, this, &AViaductPlayerController::UseHandbrake);
	InputComponent->BindAction("Handbrake", IE_Released, this, &AViaductPlayerController::StopHandbrake);
	//InputComponent->BindAction("ToggleAutopilot", IE_Released, this, &AViaductPlayerController::ToggleAutopilot);
}

void AViaductPlayerController::MoveRight(float Value)
{
	// add movement in that direction
	if (IsPossessingAVehicle())
	{
		ManualControl.Steer = AdjustSteerSensibility(Value);
		if (LKAisOn) {
			UE_LOG(LogTemp, Warning, TEXT("Value : %f / %f / %f / %f"), Value, DirectInputWheel, InstructionWheel, InstructionLevel);
			if (!HandsOnWheel) {
				//UE_LOG(LogTemp, Warning, TEXT("Value : %f / %f / %f"), DirectInputWheel, InstructionWheel, InstructionLevel);
				//HandsOnWheel = abs(Value - InstructionWheel) > 0.08;
				HandsOnWheel = abs(DirectInputWheel - InstructionWheel) > ((abs(InstructionWheel)) + 0.08);
				if (abs(InstructionWheel) < 0.08) {
					HandsOnWheel = abs(DirectInputWheel - InstructionWheel) > ((abs(InstructionWheel)) + 0.08);
				}
				if (abs(InstructionWheel) > 0.8) {
					HandsOnWheel = abs(DirectInputWheel - InstructionWheel) > ((abs(InstructionWheel) * 1.5));
					if (InstructionLevel < 0.4) {
						InstructionLevel = 30.0;
					}
					
				}
				else {
					HandsOnWheel = abs(DirectInputWheel - InstructionWheel) > ((abs(InstructionWheel) * 1.1) + 0.1 + (abs(InstructionLevel)*0.7));
					
					if (InstructionLevel > 0.0) {
						InstructionLevel -= 0.2;
					}
				}
				
			}
			
		}
	}
}

float AViaductPlayerController::AdjustSteerSensibility(float Value)
/*-1 / 1*/
{	
    if(AdjustSteering)
    {
		DirectInputWheel = Value;
        double r = (Value * 450.0 * M_PI) / 180.0; // Current input converted to radians
        double b = ((RMAX*RMAX*RMAX/STEERING_RATIO) / (MAX_WHEELS_ANGLE - RMAX/STEERING_RATIO));
        double a = b * STEERING_RATIO;
        double y = (r*r*r + b*r) / a;
        return (float)y;
    }
    else
        return Value;
}

float AViaductPlayerController::AdjustThrottle(float Val)
{
	// full release => 0.0, center value => 0.5, full pushed  => 1.0
	float a = -0.5f / ((PEDAL_RELEASED-PEDAL_MIDDLE)*abs(PEDAL_RELEASED-PEDAL_MIDDLE));
	float y = a * (Val-PEDAL_MIDDLE) * abs(Val-PEDAL_MIDDLE) + 0.5f;
	return y;
}

float AViaductPlayerController::AdjustBrake(float input)
{
	double r = PEDAL_RELEASED - PEDAL_PUSHED;
	double y = BRAKE_GAIN * (PEDAL_RELEASED-input) * (PEDAL_RELEASED-input) / (r*r);
	return (float)y;
}

void AViaductPlayerController::MoveForward(float Value)
{
	// add movement in that direction
	if (IsPossessingAVehicle())
	{
		ManualControl.Throttle = AdjustThrottle(Value);
		//UE_LOG(LogTemp, Warning, TEXT("Vrooom : %f"), adjustedValue);
	}
}

void AViaductPlayerController::Brake(float Value)
{
	// add movement in that direction
	if (IsPossessingAVehicle()  )
	{
		ManualControl.Brake = AdjustBrake(Value);
		if (LKAisOn && !HandsOnWheel) {
				HandsOnWheel = abs(Value) > 0.1;
			
		}
	}
}

void AViaductPlayerController::UseHandbrake()
{
	// set value (Disable)
	return;
	if (IsPossessingAVehicle())
	{		
		ManualControl.bHandBrake = true;
	}
}

void AViaductPlayerController::StopHandbrake()
{
	// set value (Disable)
	return;
	if (IsPossessingAVehicle())
	{
		ManualControl.bHandBrake = false;
	}
}

void AViaductPlayerController::AdaptForceFeedback(float speed_km_h)
{
#ifdef USE_G29_PLUGIN
    int force = speed_km_h / 2;
    if(force>FF_STRENGTH_MAX) force = FF_STRENGTH_MAX;
    if(force<FF_STRENGTH_MIN) force = FF_STRENGTH_MIN;
    if(UKismetMathLibrary::Abs(force-iLastForceFeedback)>=FF_MIN_DIFFERENCE)
    {
        iLastForceFeedback = force;
		//UE_LOG(LogTemp, Warning, TEXT("G29 Trying to access the G29 module"));
        auto& g29_module = FG29ControlsModule::Get();
        g29_module.WheelSetForceFeedback(force);
    }
#endif
}

void AViaductPlayerController::InitWheelController()
{
/*#ifdef USE_LOGITECH_SDK
	bool bIsConnected = ULogitechBPLibrary::WheelIsConnected(0);

	if (bIsConnected)
	{
		ULogitechBPLibrary::WheelShutdown();
		ULogitechBPLibrary::WheelInit(true);
		TryToAddSpringForce();
		//UE_LOG(LogTemp, Warning, TEXT("Good !"));
	}
#endif*/
}

void AViaductPlayerController::StopWheelController()
{
/*#ifdef USE_LOGITECH_SDK
	bool bIsConnected = ULogitechBPLibrary::WheelIsConnected(0);

	if (bIsConnected)
	{
		ULogitechBPLibrary::WheelShutdown();
	}
#endif*/
}



// =============================================================================
// -- AController --------------------------------------------------------------
// =============================================================================

void AViaductPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	if (aPawn == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Valid Pawn !"));
		return;
	}

	ACarlaWheeledVehicle* castedPawn = Cast<ACarlaWheeledVehicle>(aPawn);
	
	if (castedPawn == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Carla Wheeled Vehicle Pawn !"));
		return;
	}
	/*else
		UE_LOG(LogTemp, Warning, TEXT("P O S S E S S E D !"));*/

	Vehicle = castedPawn;

	MaximumSteerAngle = Vehicle->GetMaximumSteerAngle();
	check(MaximumSteerAngle > 0.0f);

	InitWheelController();

	//Vehicle->GetAutopilotComponent()->OnAutopilotDelegate.AddDynamic(this, &AViaductPlayerController::SetAutopilot);
	StretchWindow();
}

void AViaductPlayerController::OnUnPossess()
{
	StopWheelController();

	//if (IsPossessingAVehicle())
	//	Vehicle->GetAutopilotComponent()->OnAutopilotDelegate.RemoveDynamic(this, &AViaductPlayerController::SetAutopilot);
	
	Vehicle = nullptr;
	Super::OnUnPossess();    
}

void AViaductPlayerController::Tick(const float DeltaTime)
{
    Super::Tick(DeltaTime);

	if (!IsPossessingAVehicle())
		return;

	CacheForwardSpeed = Vehicle->GetVehicleForwardSpeed();	
	Vehicle->SetControl(ManualControl);
	
	//Vehicle->ComputeFakeAutoPilot();
	if (!bControlIsSticky)
		Vehicle->ComputeAutoPilot();
	else
		Vehicle->FlushVehicleControl();
	//UE_LOG(LogTemp, Warning, TEXT("Autopilot"));
	
	//UE_LOG(LogTemp, Warning, TEXT("Current Speed : %f"), CacheForwardSpeed * 0.036f);
	AudioComponent->SetFloatParameter("SPEED_KMH", CacheForwardSpeed * 0.036f);
    AdaptForceFeedback(CacheForwardSpeed * 0.036f);
}

void AViaductPlayerController::BeginPlay()
{
	Super::BeginPlay(); // this line right here!

	// Get a reference to the controller
	APlayerController* PController = GetWorld()->GetFirstPlayerController();
	AViaductPlayerController* castedPC = Cast<AViaductPlayerController>(PController);

	if (castedPC == this)
		StretchWindow();
}

void AViaductPlayerController::StretchWindow()
{
#if PLATFORM_LINUX
    return;
#endif
	int32 stretch_Left = 0;
	int32 stretch_Top = 0;

	int32 stretch_Width = 0;
	int32 stretch_Height = 0;

	FDisplayMetrics DisplayMetrics;
	FDisplayMetrics::GetDisplayMetrics(DisplayMetrics);

	// Start monitor on the main screen
	for (int32 Index = 0; Index < DisplayMetrics.MonitorInfo.Num(); Index++)
	{
		const FMonitorInfo Monitor = DisplayMetrics.MonitorInfo[Index];

		if (Monitor.WorkArea.Left < stretch_Left)
			stretch_Left = Monitor.DisplayRect.Left;

		if (Monitor.WorkArea.Top < stretch_Top)
			stretch_Top = Monitor.WorkArea.Top;

		int32 monitorWidth = Monitor.WorkArea.Right - Monitor.WorkArea.Left;
		int32 monitorHeight = Monitor.WorkArea.Bottom - Monitor.WorkArea.Top;

		stretch_Width += monitorWidth;

		if (monitorHeight > stretch_Height)
			stretch_Height = monitorHeight;

		UE_LOG(LogTemp, Warning, TEXT("Monitor Info (%d) : %d x %d at %d ; %d"), Index, monitorWidth, monitorHeight, Monitor.WorkArea.Left, Monitor.WorkArea.Top);
	}
#if PLATFORM_LINUX
    stretch_Width  = 5693;
    stretch_Height = 1024;
#else
	stretch_Width = 5760;
	stretch_Height = 1080;
#endif
	UE_LOG(LogTemp, Warning, TEXT("Total Screen Size : %d x %d"), stretch_Width, stretch_Height);

	const FVector2D newPosition(static_cast<float>(stretch_Left), static_cast<float>(stretch_Top));
	const FVector2D newSize(static_cast<float>(stretch_Width), static_cast<float>(stretch_Height));
	
	UE_LOG(LogTemp, Warning, TEXT("Screen Start : %d x %d"), stretch_Left, stretch_Top);

	EWorldType::Type eType = GetWorld()->WorldType;

	if (eType == EWorldType::Game)
	{
		if (GEngine)
		{
			UE_LOG(LogTemp, Warning, TEXT("Reshaping : %f x %f at %f ; %f"), newSize.X, newSize.Y, newPosition.X, newPosition.Y);
			FSystemResolution::RequestResolutionChange(newSize.X, newSize.Y, EWindowMode::Windowed);

		}
	}
}
