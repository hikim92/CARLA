// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
#include "DirectInputFFB.h"
#include "DirectInputFFBResources.h"

#define LOCTEXT_NAMESPACE "FDirectInputFFBPlugin"

DEFINE_LOG_CATEGORY(DirectInputFFBPluginLog);

void FDirectInputFFBPlugin::StartupModule()
{
	UE_LOG(DirectInputFFBPluginLog, Warning, TEXT("Initiated!"));
	IModularFeatures::Get().RegisterModularFeature(IInputDeviceModule::GetModularFeatureName(), this);
}

void FDirectInputFFBPlugin::ShutdownModule()
{
	UE_LOG(DirectInputFFBPluginLog, Warning, TEXT("Shut down!"));
	UDirectInputFFBBPLibrary::SafeReleaseDevice();
	IModularFeatures::Get().UnregisterModularFeature(IInputDeviceModule::GetModularFeatureName(), this);
}


TSharedPtr< class IInputDevice > FDirectInputFFBPlugin::CreateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler)
{
	UE_LOG(DirectInputFFBPluginLog, Log, TEXT("DirectInputFFBPlugin : CreateInputDevice"));
	
	//Expose all of our input mapping keys to the engine
	EKeys::AddKey(FKeyDetails(WheelAxis, LOCTEXT("Wheel", "DirectInputFFB Wheel"), FKeyDetails::Axis1D));
	EKeys::AddKey(FKeyDetails(AccelerationAxis, LOCTEXT("Accelerator", "DirectInputFFB Acceleration"), FKeyDetails::Axis1D));
	EKeys::AddKey(FKeyDetails(BrakeAxis, LOCTEXT("Brake", "DirectInputFFB Brake"), FKeyDetails::Axis1D));
	EKeys::AddKey(FKeyDetails(ClutchAxis, LOCTEXT("Clutch", "DirectInputFFB Clutch"), FKeyDetails::Axis1D));
	
	EKeys::AddKey(FKeyDetails(Button01, LOCTEXT("Button01", "DirectInputFFB B_01"), FKeyDetails::GamepadKey));
	EKeys::AddKey(FKeyDetails(Button02, LOCTEXT("Button02", "DirectInputFFB B_02"), FKeyDetails::GamepadKey));
	EKeys::AddKey(FKeyDetails(Button03, LOCTEXT("Button03", "DirectInputFFB B_03"), FKeyDetails::GamepadKey));
	EKeys::AddKey(FKeyDetails(Button04, LOCTEXT("Button04", "DirectInputFFB B_04"), FKeyDetails::GamepadKey));
	EKeys::AddKey(FKeyDetails(Button05, LOCTEXT("Button05", "DirectInputFFB B_05"), FKeyDetails::GamepadKey));
	EKeys::AddKey(FKeyDetails(Button06, LOCTEXT("Button06", "DirectInputFFB B_06"), FKeyDetails::GamepadKey));

	EKeys::AddKey(FKeyDetails(ButtonForward, LOCTEXT("ButtonForward", "DirectInputFFB B_Forward"), FKeyDetails::GamepadKey));
	EKeys::AddKey(FKeyDetails(ButtonBackward, LOCTEXT("ButtonBackward", "DirectInputFFB B_Backward"), FKeyDetails::GamepadKey));

	_device = new FDirectInputDevice(InMessageHandler);

	return MakeShareable(_device);
}


void FDirectInputDevice::SendControllerEvents()
{
	if (!g_pDevice){
		return;
	}
	DIJOYSTATE2 stateCurrent;
	HRESULT hr = g_pDevice->GetDeviceState(sizeof(DIJOYSTATE2), &stateCurrent);
	if (FAILED(hr)) {
		return;
	}

	// Axis Event
	currentSteeringWheel = float((stateCurrent.lX - 32767.5f) / 32767.5f);  // -1 / 1
	currentAccelerator = 1.f - float(stateCurrent.lY / 65535.f);			// 0 / 1
	currentBrake = 1.f -  float(stateCurrent.lRz / 65535.f);				// 0 / 1
	currentClutch = 1.f - float(stateCurrent.rglSlider[0] / 65535.f);		// 0 / 1

	SendAxisEvent(WheelAxis, currentSteeringWheel);
	SendAxisEvent(AccelerationAxis, currentAccelerator);
	SendAxisEvent(BrakeAxis, currentBrake);
	SendAxisEvent(ClutchAxis, currentClutch);

	// ReleaseKey
	if (currentB_01 && !stateCurrent.rgbButtons[8]) { currentB_01 = SendButtonReleaseEvent(Button01); }
	if (currentB_02 && !stateCurrent.rgbButtons[0]) { currentB_02 = SendButtonReleaseEvent(Button02); }
	if (currentB_03 && !stateCurrent.rgbButtons[1]) { currentB_03 = SendButtonReleaseEvent(Button03); }
	if (currentB_04 && !stateCurrent.rgbButtons[9]) { currentB_04 = SendButtonReleaseEvent(Button04); }
	if (currentB_05 && !stateCurrent.rgbButtons[3]) { currentB_05 = SendButtonReleaseEvent(Button05); }
	if (currentB_06 && !stateCurrent.rgbButtons[2]) { currentB_06 = SendButtonReleaseEvent(Button06); }

	if (currentB_Forward && !stateCurrent.rgbButtons[28]) { currentB_Forward = SendButtonReleaseEvent(ButtonForward); }
	if (currentB_Backward && !stateCurrent.rgbButtons[29]) { currentB_Backward = SendButtonReleaseEvent(ButtonBackward); }

	// DownKey
	if (!currentB_01 && stateCurrent.rgbButtons[8]) { currentB_01 = SendButtonDownEvent(Button01); }
	if (!currentB_02 && stateCurrent.rgbButtons[0]) { currentB_02 = SendButtonDownEvent(Button02); }
	if (!currentB_03 && stateCurrent.rgbButtons[1]) { currentB_03 = SendButtonDownEvent(Button03); }
	if (!currentB_04 && stateCurrent.rgbButtons[9]) { currentB_04 = SendButtonDownEvent(Button04); }
	if (!currentB_05 && stateCurrent.rgbButtons[3]) { currentB_05 = SendButtonDownEvent(Button05); }
	if (!currentB_06 && stateCurrent.rgbButtons[2]) { currentB_06 = SendButtonDownEvent(Button06); }

	if (!currentB_Forward && stateCurrent.rgbButtons[28]) { currentB_Forward = SendButtonDownEvent(ButtonForward); }
	if (!currentB_Backward && stateCurrent.rgbButtons[29]) { currentB_Backward = SendButtonDownEvent(ButtonBackward); }

	// More Information see : https://github.com/HWfoxtail/Unreal-Engine-Logitech-Wheel-Plugin/blob/master/Source/LogitechWheelPlugin/Private/LogitechWheelInputDevice.cpp

}

void FDirectInputDevice::SendAxisEvent(const FKey inputKey, float value) {

	//UE_LOG(DirectInputFFBPluginLog, Warning, TEXT("SendControllerEvents: Value %f."),  value);
	FAnalogInputEvent AnalogInputEvent(inputKey, FSlateApplication::Get().GetModifierKeys(), 0, 0, 0, 0, value);
	FSlateApplication::Get().ProcessAnalogInputEvent(AnalogInputEvent);
}

bool FDirectInputDevice::SendButtonDownEvent(FKey button)
{
	//UE_LOG(DirectInputFFBPluginLog, Warning, TEXT("SendButtonDownEvent"));
	FKeyEvent keyEvent(button, FSlateApplication::Get().GetModifierKeys(), 0, 0, 0, 0);
	FSlateApplication::Get().ProcessKeyDownEvent(keyEvent);
	return true;
}

bool FDirectInputDevice::SendButtonReleaseEvent(FKey button)
{
	//UE_LOG(DirectInputFFBPluginLog, Warning, TEXT("SendButtonReleaseEvent"));
	FKeyEvent keyEvent(button, FSlateApplication::Get().GetModifierKeys(), 0, 0, 0, 0);
	FSlateApplication::Get().ProcessKeyUpEvent(keyEvent);
	return false;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDirectInputFFBPlugin, DirectInputFFB);
