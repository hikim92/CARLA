// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Vehicle/VehicleControl.h"
#include "Vehicle/VehicleInputPriority.h"
#include "WheeledVehicleMovementComponent4W.h"
#include "ViaductAutopilotComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAutopilotDelegate, bool, bAutopilot);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CARLA_API UViaductAutopilotComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UViaductAutopilotComponent();
	FVehicleControl ComputeVehicleControl();
	FVehicleControl ComputeFakeVehicleControl();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void TryToGetMovementComponent();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintAssignable, Category = "Test")
	FAutopilotDelegate OnAutopilotDelegate;

	UFUNCTION(Category = "Viaduct Autopilot Component", BlueprintCallable)
	void ActivateAutopilot(bool bActivate);

	/// Set the next goal destination in front of this vehicle
	UFUNCTION(Category = "Viaduct Autopilot Component", BlueprintCallable)
	void SetAutopilotGoal(const FVector& location, float speed);

	UPROPERTY()
	UWheeledVehicleMovementComponent4W* MovementComponent = nullptr;

	UPROPERTY()
	bool ForceVehicleControl = false;

	UPROPERTY()
	FVehicleControl lastVehicleControl;

	UPROPERTY()
	EVehicleInputPriority lastPriority;

private:
	FVector goalLocation;
	float goalSpeed = 0.0;
};
