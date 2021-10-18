// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "IInputDeviceModule.h"
#include "IInputDevice.h"
#include "Modules/ModuleManager.h"
#include "Math/UnrealMathUtility.h"

#include "Core.h"
DECLARE_LOG_CATEGORY_EXTERN(DirectInputFFBPluginLog, Log, All);

#include "DirectInputFFB/Public/DirectInputFFBResources.h"
#include "DirectInputFFB/Public/DirectInputFFBBPLibrary.h"

#define SAFE_DELETE_FFB(p)  { if(p) { delete (p);     (p)=nullptr; } }
#define SAFE_RELEASE_FFB(p) { if(p) { (p)->Release(); (p)=nullptr; } }

class DIRECTINPUTFFB_API  IDirectInputFFBDevice : public IInputDeviceModule
{
public :
	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline IDirectInputFFBDevice& Get()
	{
		return FModuleManager::LoadModuleChecked< IDirectInputFFBDevice >("DirectInputFFBPlugin");
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("DirectInputFFBPlugin");
	}

	//These are typically called by a wrapped class such as LeapController (Actor Component)

	/** Get raw reference to the data and settings toggles*/
	//virtual struct LeapControllerData* ControllerData() { return nullptr; };

	/** Attach an event delegate to the leap input device loop*/
	virtual void AddEventDelegate(UObject* EventDelegate) {};

	/** Remove an event delegate from the leap input device loop*/
	virtual void RemoveEventDelegate(UObject* EventDelegate) {};

};

class FDirectInputDevice : public IInputDevice
{
public:
	FDirectInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler):MessageHandler(InMessageHandler) {};
	virtual ~FDirectInputDevice() {};

	
	/** Tick the interface (e.g. check for new controllers) */
	virtual void Tick(float DeltaTime) override {};

	/** Poll for controller state and send events if needed */
	virtual void SendControllerEvents() override;

	/** Set which MessageHandler will get the events from SendControllerEvents. */
	virtual void SetMessageHandler(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler) override {};

	/** Exec handler to allow console commands to be passed through for debugging */
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override { return false; };

	/** IForceFeedbackSystem pass through functions **/
	virtual void SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value) override {};
	virtual void SetChannelValues(int32 ControllerId, const FForceFeedbackValues &values) override {};

	TSharedRef< FGenericApplicationMessageHandler > MessageHandler;

private:
	void SendAxisEvent(const FKey inputKey, float value);
	bool SendButtonDownEvent(const FKey button);
	bool SendButtonReleaseEvent(const FKey button);
};

class FDirectInputFFBPlugin : public IDirectInputFFBDevice
{

public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual TSharedPtr<class IInputDevice> CreateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override;

private:
	FDirectInputDevice* _device;
};
