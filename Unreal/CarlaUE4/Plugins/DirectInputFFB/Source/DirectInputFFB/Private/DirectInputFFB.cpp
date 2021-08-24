// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
#include "../Public/DirectInputFFB.h"

#define LOCTEXT_NAMESPACE "FDirectInputFFBPlugin"

void FDirectInputFFBPlugin::StartupModule()
{
	UE_LOG(LogTemp, Warning, TEXT("DirectInputFFBMPlugin initiated!"));
}

void FDirectInputFFBPlugin::ShutdownModule()
{
	UE_LOG(LogTemp, Warning, TEXT("DirectInputFFBMPlugin shut down!"));
	UDirectInputFFBBPLibrary::SafeReleaseDevice();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDirectInputFFBPlugin, DirectInputFFB);
