// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "HUDValues.generated.h"

USTRUCT(BlueprintType)
struct CARLA_API FHUDValues
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 flags = 0;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 value1 = 0;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 value2 = 0;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 value3 = 0;
};
