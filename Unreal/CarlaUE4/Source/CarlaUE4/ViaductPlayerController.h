// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Vehicle/VehicleControl.h"
#include "Vehicle/VehicleInputPriority.h"
#include "GameFramework/PlayerController.h"
#include "Carla/Settings/HUDValues.h"
#include "ViaductPlayerController.generated.h"


class UWheeledVehicleMovementComponent4W;
class ACarlaWheeledVehicle;
class UAudioComponent;

#define PEDAL_RELEASED 0.0f
#define PEDAL_MIDDLE   0.5f
#define PEDAL_PUSHED   1.0f


#if !defined(M_PI)
 #define M_PI 3.14159265358979323846f
#endif  
#define RMAX ((450.0f * M_PI) / 180.0f) // Max input in radians

#define STEERING_RATIO   45.f     // Steering wheel ratio  1:15 -> 1:50
#define MAX_WHEELS_ANGLE 0.7f  // Max angle of the car wheels in radians (audi TT=0.7)

#define BRAKE_GAIN       0.7f

#define PEDAL_R					((PEDAL_RELEASED - PEDAL_PUSHED) * (PEDAL_RELEASED - PEDAL_PUSHED))
#define PEDAL_A					( -0.5f / ((PEDAL_RELEASED - PEDAL_MIDDLE)* abs(PEDAL_RELEASED - PEDAL_MIDDLE)))
#define STEERING_RATIO_COMPUTED ((RMAX * RMAX * RMAX / STEERING_RATIO) / (MAX_WHEELS_ANGLE - RMAX / STEERING_RATIO))
#define STEERING_FINAL          (STEERING_RATIO_COMPUTED * STEERING_RATIO)

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
	void SetStickyControl(bool bEnabled)
	{
		bControlIsSticky = bEnabled;
	}
	
	UFUNCTION(Category = "Viaduct PC", BlueprintCallable)
	void SetReverse(bool bNewReverse)
	{
		ManualControl.bReverse = bNewReverse;
	}	

	UPROPERTY(VisibleAnywhere, Category = "Viaduct PC")
	bool bControlIsSticky = false;

	/*
	HUDValues.flags : 
	
	HUD_MIRROR_W_LEFT = & 2;
	HUD_MIRROR_W_RIGHT = & 4;
	HUD_FCW = & 8;
	HUD_AEB = & 16;

	HUD_LDW = & 32;
	HUD_ACC = & 64;
	HUD_LKA = & 128;
	HUD_ACC_DISABLE = & 256;

	HUD_LKA_DISABLE = & 512;
	HUD_TURN_LEFT =   & 1024;
	HUD_TURN_RIGHT =  & 2048;


	...
	*/
	UFUNCTION(Category = "Viaduct PC", BlueprintCallable)
	void SetHUDValues(FHUDValues newHudValues);

	// HUD - Autopilote
	UPROPERTY(BlueprintReadOnly, Category = "Viaduct PC")
	bool HUD_MIRROR_W_LEFT = false;

	UPROPERTY(BlueprintReadOnly, Category = "Viaduct PC")
	bool HUD_MIRROR_W_RIGHT = false;

	UPROPERTY(BlueprintReadOnly, Category = "Viaduct PC")
	bool HUD_FCW = false;

	UPROPERTY(BlueprintReadOnly, Category = "Viaduct PC")
	bool HUD_AEB = false;

	UPROPERTY(BlueprintReadOnly, Category = "Viaduct PC")
	bool HUD_LDW = false;

	UPROPERTY(BlueprintReadOnly, Category = "Viaduct PC")
	bool HUD_LKA = false;

	UPROPERTY(BlueprintReadWrite, Category = "Viaduct PC")
	bool CLEAN_PID = false;

	UPROPERTY(BlueprintReadOnly, Category = "Viaduct PC")
	bool HUD_LKA_DISABLE = false;

	UPROPERTY(BlueprintReadOnly, Category = "Viaduct PC")
	bool HUD_ACC = false;

	UPROPERTY(BlueprintReadOnly, Category = "Viaduct PC")
	bool HUD_ACC_DISABLE = false;

	UPROPERTY(BlueprintReadOnly, Category = "Viaduct PC")
	bool HUD_TURN_LEFT = false;

	UPROPERTY(BlueprintReadOnly, Category = "Viaduct PC")
	bool HUD_TURN_RIGHT = false;

	UPROPERTY(BlueprintReadOnly, Category = "Viaduct PC")
	int HUD_SPEED_LIMIT = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Viaduct PC")
	int HUD_ACC_SPEED = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Viaduct PC")
	bool HUD_BLINKER_LEFT = false;

	UPROPERTY(BlueprintReadWrite, Category = "Viaduct PC")
	bool HUD_BLINKER_RIGHT = false;

	UPROPERTY(BlueprintReadOnly, Category = "Viaduct PC")
	UAudioComponent* SOUND_FCW = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Viaduct PC")
	UAudioComponent* SOUND_AEB = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Viaduct PC")
	UAudioComponent* SOUND_LDW = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Viaduct PC")
	UAudioComponent* SOUND_BLINKER = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Viaduct PC")
	UAudioComponent* SOUND_AUTOPILOT_OFF = nullptr;

protected:
	/** Called for side to side input */
	void MoveForward(float Val);
	void MoveRight(float Val);
	void Brake(float Val);

	void UseHandbrake();
	void StopHandbrake();	

	virtual void BeginPlay() override;
	void StretchWindow();

	/** handle of active TryToAddSpringForce timer  */
	UPROPERTY()
	FTimerHandle SpringForceTimerHandle;

	UPROPERTY()
	FVehicleControl ManualControl;	

	UPROPERTY()
	UAudioComponent* SoundOutsideAmbi = nullptr;

	UPROPERTY(VisibleAnywhere)
	ACarlaWheeledVehicle* Vehicle = nullptr;

	UPROPERTY()
	FHUDValues HUDValues;

	UPROPERTY(VisibleAnywhere)
	float MaximumSteerAngle = -1.0f;

	UPROPERTY(VisibleAnywhere)
	float CacheForwardSpeed = 0.0f;

	UPROPERTY(VisibleAnywhere)
	bool AdjustSteering = true;

private:
	float AdjustSteerSensibility(float Val);
	float AdjustThrottle(float Val);
	float AdjustBrake(float Val);

	void PlaySoundHMI(UAudioComponent* audioC, bool play = false);
};
