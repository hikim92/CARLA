// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Vehicle/VehicleControl.h"
#include "Vehicle/VehicleInputPriority.h"
#include "GameFramework/PlayerController.h"
#include "ViaductPlayerController.generated.h"

class UWheeledVehicleMovementComponent4W;
class ACarlaWheeledVehicle;
class UAudioComponent;

#define PEDAL_RELEASED 0.0f
#define PEDAL_MIDDLE   0.5f
#define PEDAL_PUSHED   1.0f

#if !defined(M_PI)
  #define M_PI 3.141592f
#endif  
#define RMAX ((450.0 * M_PI) / 180.0) // Max input in radians

#define STEERING_RATIO   45     // Steering wheel ratio  1:15 -> 1:50
#define MAX_WHEELS_ANGLE (0.7)  // Max angle of the car wheels in radians (audi TT=0.7)

#define BRAKE_GAIN       0.7

#define FF_STRENGTH       30  // Default force feedback
#define FF_STRENGTH_MIN   25  // Weakest force feedback, for [0-50] km/h
#define FF_STRENGTH_MAX   50  // Strongest force feedback, for [100,...] km/h  => speed / 2
#define FF_MIN_DIFFERENCE  2  // Minimal difference to actually change the force feedback of the G29

/**
 * 
 */
UCLASS()
class CARLAUE4_API AViaductPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AViaductPlayerController(const FObjectInitializer& ObjectInitializer);
	~AViaductPlayerController();

	virtual void SetupInputComponent() override;
	void OnPossess(APawn* aPawn) override;
	void OnUnPossess() override;
	void Tick(float DeltaTime) override;

	UFUNCTION(Category = "Viaduct PC", BlueprintCallable)
	void InitWheelController();

	UFUNCTION(Category = "Viaduct PC", BlueprintCallable)
	void StopWheelController();
	
	UFUNCTION(Category = "Viaduct PC", BlueprintCallable)
	bool IsPossessingAVehicle() const
	{
		return Vehicle != nullptr;
	}

	UFUNCTION(Category = "Viaduct PC", BlueprintCallable)
	ACarlaWheeledVehicle* GetPossessedVehicle()
	{
		return Vehicle;
	}

	UFUNCTION(Category = "Viaduct PC", BlueprintCallable)
	bool GetHandsOnWheel()
	{
		if (HandsOnWheel) {
			HandsOnWheel = false;
			return true;
		}
		return HandsOnWheel;  // Always false
	}
	
	UFUNCTION(Category = "Viaduct PC", BlueprintCallable)
	float GetInstructionWheel()
	{
		return InstructionWheel;
	}

	UFUNCTION(Category = "Viaduct PC", BlueprintCallable)
	void SetInstructionWheel(float value)
	{
		InstructionWheel = FMath::Clamp(value, -1.0f, 1.0f);
	}


	UFUNCTION(Category = "Viaduct PC", BlueprintCallable)
	void SetStickyControl(bool bEnabled)
	{
		bControlIsSticky = bEnabled;
	}
	
	UFUNCTION(Category = "Viaduct PC", BlueprintCallable)
	void SetReverse(bool bNewReverse)
	{
		ManualControl.bReverse = bNewReverse;
	}	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viaduct PC")
	bool LKAisOn;

	UPROPERTY(VisibleAnywhere, Category = "Viaduct PC")
	bool bControlIsSticky = false;

	UPROPERTY(VisibleAnywhere, Category = "Viaduct PC")
	bool bSemiManualControl = true;


    int iLastForceFeedback = -1;

protected:
	/** Called for side to side input */
	void MoveForward(float Val);
	void MoveRight(float Val);
	void Brake(float Val);

	float CheckReverse(float Val);
	float AdjustSteerSensibility(float Val);
	float AdjustThrottle(float Val);
	float AdjustBrake(float Val);

    void AdaptForceFeedback(float speed);

	void UseHandbrake();
	void StopHandbrake();	
	void PlaySpringForce();		

	virtual void BeginPlay() override;
	void StretchWindow();

	float InstructionLevel = 0.0f;

	/** handle of active TryToAddSpringForce timer  */
	UPROPERTY()
	FTimerHandle SpringForceTimerHandle;

	UPROPERTY()
	FVehicleControl ManualControl;	

	UPROPERTY()
	USoundCue* AudioCue;

	UPROPERTY()
	UAudioComponent* AudioComponent = nullptr;

	UPROPERTY(VisibleAnywhere)
	ACarlaWheeledVehicle* Vehicle = nullptr;

	UPROPERTY(VisibleAnywhere)
	float MaximumSteerAngle = -1.0f;

	UPROPERTY(VisibleAnywhere)
	float CacheForwardSpeed = 0.0f;

	UPROPERTY(VisibleAnywhere)
	bool UseArcadeMode = true;

	UPROPERTY(VisibleAnywhere)
	bool AdjustSteering = true;

	UPROPERTY(EditAnywhere)
	bool HandsOnWheel = false;

	UPROPERTY(VisibleAnywhere)
	float KFlat = 1.0f;

	UPROPERTY(VisibleAnywhere)
	float KScale = 3.0f;

	UPROPERTY(EditAnywhere)
	float InstructionWheel = 0.0f;  // -1.0, 1.0

	UPROPERTY(BlueprintReadOnly)
	float DirectInputWheel = 0.0f;  // -1.0, 1.0
};
