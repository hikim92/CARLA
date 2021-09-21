// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Kismet/KismetMathLibrary.h"
#include "ViaductAutopilotComponent.h"

// Sets default values for this component's properties
UViaductAutopilotComponent::UViaductAutopilotComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UViaductAutopilotComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	TryToGetMovementComponent();	
}


// Called every frame
void UViaductAutopilotComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UViaductAutopilotComponent::TryToGetMovementComponent()
{
	UActorComponent* newComp = GetOwner()->GetComponentByClass(UWheeledVehicleMovementComponent4W::StaticClass());

	if (newComp != nullptr)
		MovementComponent = Cast<UWheeledVehicleMovementComponent4W>(newComp);
}

void UViaductAutopilotComponent::SetAutopilotGoal(const FVector& location, float speed)
{
	goalLocation = location;
	goalSpeed = speed;
}


void UViaductAutopilotComponent::ActivateAutopilot(bool bActivate)
{
	// Call Delegate to set Autopilot on Controller
	OnAutopilotDelegate.Broadcast(bActivate);
}


FVehicleControl UViaductAutopilotComponent::ComputeVehicleControl()
{
	// One possibility :
		  // v - enable this bAutoPilot - possible only with apply_batch_sync([ SpawnActor(blueprint, spawn_point).then(SetAutopilot(FutureActor, True,tm_port)) ])
		  //   - transmit a goal speed
		  //   - regularly, but not every frame, transmit a new goal waypoint (that does not have to be on a lane)
		  //   - let this function adapt the vehicle control
		  //   - ideally also disable the collisions
		  //   - the above function IsThereAnObstacleAhead can be useful for ACC
	FVehicleControl autoctrl;
#if not PLATFORM_LINUX
	float M_PI = UKismetMathLibrary::GetPI();
#endif
	if (MovementComponent != nullptr)
	{
		// Legacy
		autoctrl.bHandBrake = false;
		autoctrl.bReverse = false;
		autoctrl.bManualGearShift = false;
		autoctrl.Gear = 0;

		// Cruise control
		if (goalSpeed > 0.0)
		{
			float speed = MovementComponent->GetForwardSpeed() / 100.0;  // [cm/s] -> [m/s]
			float cmd = 0.5 - 0.2 * (speed - goalSpeed);
			if (cmd > 0.7f) cmd = 0.7f;
			if (cmd < 0.0f) cmd = 0.0f;
			//UE_LOG(LogTemp, Warning, TEXT("UViaductAutopilotComponent::ComputeVehicleControl - car speed : %f -> %f => %f"), speed, GoalSpeed, cmd);
			autoctrl.Throttle = cmd;
			autoctrl.Brake = 0;
		}
		else
		{
			if (goalSpeed >= -1.0 && goalSpeed <= 0.0) {   // Emergency braking
				autoctrl.Throttle = 0.0;
				autoctrl.Brake = 0.7 * abs(goalSpeed);
			}
			else {  // Manual mode
				autoctrl.Throttle = this->lastVehicleControl.Throttle;
				autoctrl.Brake = this->lastVehicleControl.Brake;
				autoctrl.bReverse = this->lastVehicleControl.bReverse;
			}
		}

		if (goalLocation.X >= 500000000.f && goalLocation.Y >= 500000000.f) {
			autoctrl.Steer = this->lastVehicleControl.Steer;;
		}
		else {
			// Direction following (Lane keeping and Lane change)
			FTransform current_transform = GetOwner()->GetTransform();
			float dx = goalLocation.X - current_transform.GetTranslation().X;
			float dy = goalLocation.Y - current_transform.GetTranslation().Y;
			float distance = sqrt(dx*dx + dy * dy);
			bool manual = true;
			//UE_LOG(LogTemp, Warning, TEXT("ComputeVehicleControl::distance - %f"), distance);
			if ((distance > 500.0f) && (distance < 2800.0f)) // Min and max distance for the carrot
			{
				dx /= distance; // Normalize the car-carrot vector
				dy /= distance; // Normalize the car-carrot vector
				// Is it in front of us ? Dot product >= 0.9 (cos 25 degrees)
				auto ref_dir = current_transform.GetRotation().GetForwardVector();
				float dot_product = dx * ref_dir.X + dy * ref_dir.Y;
				//UE_LOG(LogTemp, Warning, TEXT("ComputeVehicleControl::dot_product - %f"), dot_product);
				if (dot_product > 0.9)
				{	float goal_angle = atan2f(dy, dx);
					// Turn the wheel in the right direction to aim for the goal angle
					float car_angle = current_transform.GetRotation().Euler().Z * M_PI / 180.0;
					float diff_angle = goal_angle - car_angle;
					if (diff_angle > M_PI)  diff_angle -= 2 * M_PI;
					if (diff_angle < -M_PI) diff_angle += 2 * M_PI;
					float steer = 0.8185 * diff_angle; //    # max wheel angle of audi tt is 70 degrees
					autoctrl.Steer = steer;
				}
					
			}
		
		}
		
		//LEGACY : autoctrl.Brake = (GoalSpeed < 0.0) ? 0.7 : 0.0;  // Emergency braking when GoalSpeed < 0.0 
        //float rpm = MovementComponent->GetEngineRotationSpeed();
        //UE_LOG(LogTemp, Warning, TEXT("UViaductAutopilotComponent::ComputeVehicleControl - %f RPM. %d -> %d"),
        //                               rpm, MovementComponent->GetCurrentGear(), MovementComponent->GetTargetGear() );
        //UE_LOG(LogTemp, Warning, TEXT("UViaductAutopilotComponent::ComputeVehicleControl - car speed : %f -> %f => %f"), speed, GoalSpeed, cmd);
	}
	else
		TryToGetMovementComponent();

	return autoctrl;
}

FVehicleControl UViaductAutopilotComponent::ComputeFakeVehicleControl()
{
	FVehicleControl autoctrl;

	// Cruise control
	float speed = MovementComponent->GetForwardSpeed() / 100.0;  // [cm/s] -> [m/s]

	autoctrl.Throttle = 1.0f;
	autoctrl.Steer = 0.0f;

	autoctrl.Brake = 0.0f;
	autoctrl.bHandBrake = false;
	autoctrl.bReverse = false;
	autoctrl.bManualGearShift = false;
	//autoctrl.Gear = 1 + int(speed / 8.0);

	return autoctrl;
}
