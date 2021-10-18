// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "DirectInputFFBResources.h"
#include <vector>

#include "Kismet/BlueprintFunctionLibrary.h"
#include "DirectInputFFBBPLibrary.generated.h"


/* 
*	Function library class.
*	Each function in it is expected to be static and represents blueprint node that can be called in any blueprint.
*
*	When declaring function you can define metadata for the node. Key function specifiers will be BlueprintPure and BlueprintCallable.
*	BlueprintPure - means the function does not affect the owning object in any way and thus creates a node without Exec pins.
*	BlueprintCallable - makes a function which can be executed in Blueprints - Thus it has Exec pins.
*	DisplayName - full name of the node, shown when you mouse over the node and in the blueprint drop down menu.
*				Its lets you name the node using characters not allowed in C++ function names.
*	CompactNodeTitle - the word(s) that appear on the node.
*	Keywords -	the list of keywords that helps you to find node when you search for it using Blueprint drop-down menu. 
*				Good example is "Print String" node which you can find also by using keyword "log".
*	Category -	the category your node will be under in the Blueprint drop-down menu.
*
*	For more info on custom blueprint nodes visit documentation:
*	https://wiki.unrealengine.com/Custom_Blueprint_Node_Creation
*/
UCLASS()
class UDirectInputFFBBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	static void SafeReleaseDevice();

	static bool InitWheel();

	/* Init device and effects */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "DirectInputFFB Init", Keywords = "DirectInputFFB Init"), Category = "DirectInputFFB")
	static bool InitDevice();

	/* Get Steering Wheel Position
	
	inDegree : return float in inDegree, see STREERINGWHEEL_ANGLE_MAX

	return : float between (-1.0 & 1.0) or ( - STREERINGWHEEL_ANGLE_MAX/2.0 & STREERINGWHEEL_ANGLE_MAX/2.0)
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "DirectInputFFB Steering Wheel Position", Keywords = "DirectInputFFB Steering Wheel Position"), Category = "DirectInputFFB")
	static float SteeringWheelPosition(bool inDegree = false);

	/* Set Spring parameter effect
	 
	offset   (int) : clamped (-10 000 & 10 000)
	posCoef  (int) : clamped (-10 000 & 10 000)
	negCoef  (int) : clamped (-10 000 & 10 000)
	posSat   (int) : clamped (0 & 10 000)
	negSat   (int) : clamped (0 & 10 000)
	deadZone (int) : clamped (0 & 10 000)
	
	For more information : https://docs.microsoft.com/en-us/previous-versions/windows/desktop/ee416601(v=vs.85)
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "DirectInputFFB Set Param Sprint", Keywords = "DirectInputFFB Spring"), Category = "DirectInputFFB")
	static bool SetSpringParameters(int offset = 0, int posCoef = 10000, int negCoef = 10000, int posSat = 10000, int negSat = 10000, int deadZone = 0);

	/* Set SinWave parameter effect
	
	offset    (int) : clamped (-10 000 & 10 000)
	magnitude (int) : clamped (0 & 10 000)
	phase     (int) : clamped (0 & 35 999)
	period    (int) : microseconds

	For more information : https://docs.microsoft.com/en-us/previous-versions/windows/desktop/ee416634(v=vs.85)
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "DirectInputFFB Set Param SinWave", Keywords = "DirectInputFFB SinWave"), Category = "DirectInputFFB")
	static bool SetSinWaveParameters(int offset = 0, int magnitude = 0, int phase = 0, int period = 10000);

	/* Set Constant force parameter effect

	force    (int) : clamped (-MAX_TORQUE & MAX_TORQUE)

	For more information : https://docs.microsoft.com/en-us/previous-versions/windows/desktop/ee416604(v=vs.85)
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "DirectInputFFB Set ConstantForce", Keywords = "DirectInputFFB ConstantForce"), Category = "DirectInputFFB")
	static bool SetConstantForceParameters(int force);

	/* Set PID constant force parameters
	
	consigne (float) : new consigne in degree
	handsOnWheel (bool) : return true if Hands On Wheel is detected
	kp (float) : P
	ki (float) : I
	kd (float) : D
	deltaTime (float) : delta time in second
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "DirectInputFFB Set PID ConstantForce", Keywords = "DirectInputFFB Set PID ConstantForce"), Category = "DirectInputFFB")
	static bool SetPIDConstantForceParameters(float consigne, bool& handsOnWheel, float kp = -50.0f, float ki = -8.0f, float kd = -2.8f, float speedVehicle=10.0, bool lkaIsOn=false, float deltaTime = 0.02);

	/* Clean values for PID */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "DirectInputFFB Clean PID values", Keywords = "DirectInputFFB Clean PID values"), Category = "DirectInputFFB")
	static void CleanPIDValues();

	/* Set the streering wheel position 
	
	consigne (float) : consigne in degree

	During 1 sec
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "DirectInputFFB Set Init position streering wheel", Keywords = "DirectInputFFB Set Init SteeringWheel"), Category = "DirectInputFFB")
	static bool SetInitPositionStreeringWheel(float consigne);

	static bool SetDamperParameters(int posSat);

private:

	static std::vector<float> Hist_position;
	static std::vector<float> Hist_goal;
	static std::vector<LPDIRECTINPUTDEVICE8> Devices;
	static int currentDevice;
	static bool enableDamper;

	template<typename T>
	static T clip(const T& value, const T& min, const T& max) {
		return std::max(min, std::min(value, max));
	}

	static BOOL CALLBACK EnumFFDevicesCallback(const DIDEVICEINSTANCE* pInst, VOID* pContext)
	{
		LPDIRECTINPUTDEVICE8 pDevice;
		HRESULT hr;

		// Obtain an interface to the enumerated force feedback device.
		hr = g_pDI->CreateDevice(pInst->guidInstance, &pDevice, nullptr);

		// If it failed, then we can't use this device for some
		// bizarre reason.  (Maybe the user unplugged it while we
		// were in the middle of enumerating it.)  So continue enumerating
		if (FAILED(hr))
			return DIENUM_CONTINUE;

		// We successfully created an IDirectInputDevice8.  So stop looking 
		// for another one.
		UDirectInputFFBBPLibrary::Devices.push_back(pDevice);
		return DIENUM_CONTINUE;
	}

};
