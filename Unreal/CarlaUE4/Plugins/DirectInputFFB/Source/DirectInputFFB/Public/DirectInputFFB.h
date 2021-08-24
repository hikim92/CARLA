// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Modules/ModuleManager.h"

#include "DirectInputFFB/Public/DirectInputFFBResources.h"
#include "DirectInputFFB/Public/DirectInputFFBBPLibrary.h"

#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=nullptr; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=nullptr; } }

class FDirectInputFFBPlugin : public IModuleInterface
{

public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

};
