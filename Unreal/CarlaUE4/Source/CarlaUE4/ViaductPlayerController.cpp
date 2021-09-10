// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaUE4.h"
#include "ViaductPlayerController.h"
#include "Vehicle/CarlaWheeledVehicle.h"
#include "Vehicle/ViaductAutopilotComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/KismetMathLibrary.h"
	
// =============================================================================
// -- Constructor and destructor -----------------------------------------------
// =============================================================================

AViaductPlayerController::AViaductPlayerController(const FObjectInitializer& ObjectInitializer)	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	FApp::SetUnfocusedVolumeMultiplier(1.0f);

	// Fetch our ambient Sound Cue ==========================================================
	static ConstructorHelpers::FObjectFinder<USoundCue> AudioCueAsset(TEXT("'/Game/Viaduct/Sounds/Vehicle/Road-Ride_Cue.Road-Ride_Cue'"));
	SoundOutsideAmbi = CreateDefaultSubobject<UAudioComponent>("AudioComp");
	AddOwnedComponent(SoundOutsideAmbi);
	SoundOutsideAmbi->SetSound(AudioCueAsset.Object);
	SoundOutsideAmbi->SetAutoActivate(true);

	// HMI Sound =============================================================================
	static ConstructorHelpers::FObjectFinder<USoundWave> soundFCW(TEXT("'/Game/Viaduct/HMI/Sounds/FCW'"));
	SOUND_FCW = CreateDefaultSubobject<UAudioComponent>("AudioCompFCW");
	AddOwnedComponent(SOUND_FCW);
	SOUND_FCW->SetSound(soundFCW.Object);
	SOUND_FCW->SetAutoActivate(false);

	static ConstructorHelpers::FObjectFinder<USoundWave> soundAEB(TEXT("'/Game/Viaduct/HMI/Sounds/AEB'"));
	SOUND_AEB = CreateDefaultSubobject<UAudioComponent>("AudioCompAEB");
	AddOwnedComponent(SOUND_AEB);
	SOUND_AEB->SetSound(soundFCW.Object);
	SOUND_AEB->SetAutoActivate(false);

	static ConstructorHelpers::FObjectFinder<USoundWave> soundLDW(TEXT("'/Game/Viaduct/HMI/Sounds/LDW'"));
	SOUND_LDW = CreateDefaultSubobject<UAudioComponent>("AudioCompLDW");
	AddOwnedComponent(SOUND_LDW);
	SOUND_LDW->SetSound(soundFCW.Object);
	SOUND_LDW->SetAutoActivate(false);

	static ConstructorHelpers::FObjectFinder<USoundWave> soundBlinker(TEXT("'/Game/Viaduct/HMI/Sounds/BLINKER'"));
	SOUND_BLINKER = CreateDefaultSubobject<UAudioComponent>("AudioCompBlinker");
	AddOwnedComponent(SOUND_BLINKER);
	SOUND_BLINKER->SetSound(soundBlinker.Object);
	SOUND_BLINKER->SetAutoActivate(false);

	static ConstructorHelpers::FObjectFinder<USoundWave> soundAutopilot(TEXT("'/Game/Viaduct/HMI/Sounds/AutopilotOff'"));
	SOUND_AUTOPILOT_OFF = CreateDefaultSubobject<UAudioComponent>("AudioCompAutoPilot");
	AddOwnedComponent(SOUND_AUTOPILOT_OFF);
	SOUND_AUTOPILOT_OFF->SetSound(soundAutopilot.Object);
	SOUND_AUTOPILOT_OFF->SetAutoActivate(false);

	// Control =============================================================================
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
		//UE_LOG(LogTemp, Warning, TEXT("MoveRight : %f"), Value);
		ManualControl.Steer = AdjustSteerSensibility(Value);
	}
}

float AViaductPlayerController::AdjustSteerSensibility(float Value)
/*-1 / 1*/
{	
    if(AdjustSteering)
    {
		float r = (Value * 450.0 * M_PI) / 180.0; // Current input converted to radians		
        return (float)(r * r * r + STEERING_RATIO_COMPUTED * r) / STEERING_FINAL;
    }
    return Value;
}

float AViaductPlayerController::AdjustThrottle(float Val)
{
	// full release => 0.0, center value => 0.5, full pushed  => 1.0
	return (float)PEDAL_A * (Val - PEDAL_MIDDLE) * abs(Val - PEDAL_MIDDLE) + 0.5f;
}

float AViaductPlayerController::AdjustBrake(float input)
{
	return (float)BRAKE_GAIN * (PEDAL_RELEASED - input) * (PEDAL_RELEASED - input) / PEDAL_R;;
}

void AViaductPlayerController::MoveForward(float Value)
{
	// add movement in that direction
	if (IsPossessingAVehicle())
	{
		//UE_LOG(LogTemp, Warning, TEXT("MoveForward : %f"), Value);
		ManualControl.Throttle = AdjustThrottle(Value);
	}
}

void AViaductPlayerController::Brake(float Value)
{
	// add movement in that direction
	if (IsPossessingAVehicle()  )
	{
		//UE_LOG(LogTemp, Warning, TEXT("Brake : %f"), Value);
		ManualControl.Brake = AdjustBrake(Value);
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


// =============================================================================
// -- AController --------------------------------------------------------------
// =============================================================================

void AViaductPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);
	
	if (aPawn == nullptr) {
		return;
	}

	ACarlaWheeledVehicle* castedPawn = Cast<ACarlaWheeledVehicle>(aPawn);
	
	if (castedPawn == nullptr) {
		return;
	}

	Vehicle = castedPawn;

	MaximumSteerAngle = Vehicle->GetMaximumSteerAngle();
	check(MaximumSteerAngle > 0.0f);

	//Vehicle->GetAutopilotComponent()->OnAutopilotDelegate.AddDynamic(this, &AViaductPlayerController::SetAutopilot);
	StretchWindow();
}

void AViaductPlayerController::OnUnPossess()
{
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
	SoundOutsideAmbi->SetFloatParameter("SPEED_KMH", CacheForwardSpeed * 0.036f);	
}

void AViaductPlayerController::BeginPlay()
{
	Super::BeginPlay();
	StretchWindow();
	// Get a reference to the controller
	APlayerController* PController = GetWorld()->GetFirstPlayerController();
	AViaductPlayerController* castedPC = Cast<AViaductPlayerController>(PController);

	if (castedPC == this)
		StretchWindow();
}

void AViaductPlayerController::SetHUDValues(FHUDValues newHudValues)
{
	HUDValues = newHudValues;

	HUD_MIRROR_W_LEFT = HUDValues.flags & 2;
	HUD_MIRROR_W_RIGHT = HUDValues.flags & 4;
	HUD_FCW = HUDValues.flags & 8;
	HUD_AEB = HUDValues.flags & 16;
	HUD_LDW = HUDValues.flags & 32;
	HUD_ACC = HUDValues.flags & 64;

	// Autopilot 
	if (bool(HUDValues.flags & 128) != HUD_LKA) { 
		CLEAN_PID = true; 
		if(HUD_LKA){ // This case mean : Lost Autopilot
			PlaySoundHMI(SOUND_AUTOPILOT_OFF, true); 
		}
	}
	HUD_LKA = HUDValues.flags & 128;
	if(HUD_LKA){ ENABLE_HOW = true; }

	HUD_ACC_DISABLE = HUDValues.flags & 256;
	HUD_LKA_DISABLE = HUDValues.flags & 512;
	HUD_TURN_LEFT = HUDValues.flags & 1024;
	HUD_TURN_RIGHT = HUDValues.flags & 2048;

	// TODO : Wait QT-Scenario-Player feature
	//HUD_BLINKER_LEFT = HUDValues.flags & 4096;
	//HUD_BLINKER_RIGHT = HUDValues.flags & 8192;

	HUD_SPEED_LIMIT = HUDValues.value2;
	HUD_ACC_SPEED = HUDValues.value3;

	PlaySoundHMI(SOUND_FCW, HUD_FCW);
	PlaySoundHMI(SOUND_AEB, HUD_AEB);
	PlaySoundHMI(SOUND_LDW, HUD_LDW);

	PlaySoundHMI(SOUND_BLINKER, HUD_BLINKER_LEFT);
	PlaySoundHMI(SOUND_BLINKER, HUD_BLINKER_RIGHT);
}

void AViaductPlayerController::SetHandsOnWheelDetected()
{
	if (HUD_LKA) {
		CLEAN_PID = true;
		ENABLE_HOW = false;
		PlaySoundHMI(SOUND_AUTOPILOT_OFF, true);
	}
}

void AViaductPlayerController::PlaySoundHMI(UAudioComponent* audioC, bool play)
{
	if (play && !audioC->IsPlaying()) { audioC->Play(); }
}

void AViaductPlayerController::StretchWindow()
{
	UE_LOG(LogTemp, Warning, TEXT("Monitor StretchWindow"));
	GEngine->Exec(GetWorld(), TEXT("r.ScreenPercentage 80"));
	GEngine->Exec(GetWorld(), TEXT("r.setres 5760x1080")); // r.ScreenPercentage 100
}
