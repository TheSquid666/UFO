// Fill out your copyright notice in the Description page of Project Settings.


#include "SteeringAIController.h"
#include "Engine/TargetPoint.h"
#include "UFOPawn.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

ASteeringAIController::ASteeringAIController() 
	: MyUfo{ nullptr }
	, SteeringVelocity{FVector::ZeroVector}
	, SteeringQuat{ FQuat::Identity }
	, bTargetMoved{ true }
	, bDebugAll{true}
	, bDebugRotation{ true }
	, bDebugMovement{ true }
	, DebugOnScreenTime{0.03f}
	, Target{nullptr}
	, TargetAngleReachedTolerance{ 2.f }
	, TargetReachedRadius{5.f}
	, TargetSlowRadius{500.f}
	, TimeToTarget{0.1f}
	, RollScale{ -0.4f }
	, bAddRollOnTurns {true}
	, NavStatus{ ENS_Stopped }
{}

void ASteeringAIController::OnPossess(APawn* InPawn)
{
	//Since we don't want to use a movement component, get the ufo object to call it's method directly
	MyUfo = dynamic_cast<AUFOPawn*>(InPawn);
	if (MyUfo != nullptr)
	{
		Target = MyUfo->GetTarget();
	}
}

void ASteeringAIController::OnUnPossess()
{
	MyUfo = nullptr;
	Target = nullptr;
}

/*
* Steering State Machine
* The brain
*/
void ASteeringAIController::UpdateState(float DeltaTime)
{
	if (Target == nullptr || MyUfo == nullptr)
	{
		StopMovement();
		return;
	}

	switch (NavStatus)
	{
	case ENavStatus::ENS_Stopped:
		//We have a target, face it
		if (bTargetMoved)
		{
			bTargetMoved = false;
			NavStatus = ENavStatus::ENS_Moving;
		}
		break;
	case ENavStatus::ENS_Moving:
	{
		if (Arrive())
		{
			//We have arrived within radius
			NavStatus = ENavStatus::ENS_Aligning_yaw;
		}
	}
	break;
	case ENavStatus::ENS_Aligning_yaw:
	{
		//Align with the desired orientation
		if (AlignYaw())
		{
			NavStatus = ENavStatus::ENS_Aligning_pitch;
		}
	}
	break;
	case ENavStatus::ENS_Aligning_pitch:
	{
		//Align with the desired orientation
		if (AlignPitch())
		{
			NavStatus = ENavStatus::ENS_Aligning_roll;
		}
	}
	break;
	case ENavStatus::ENS_Aligning_roll:
	{
		//Align with the desired orientation
		if (AlignRoll())
		{
			NavStatus = ENavStatus::ENS_Stopped;
			StopMovement();
		}
	}
	break;
	}
}

//Generic method for all "align", curent and expected values (yaw, pitch roll) and the vector to make the Quat
bool ASteeringAIController::Align3D(float currentVal, float expectedVal, const FVector& quatVector)
{
	if (!FMath::IsNearlyEqual(currentVal, expectedVal, TargetAngleReachedTolerance))
	{
		float delta = FMath::DegreesToRadians(expectedVal - currentVal);
		SteeringQuat *= FQuat{ quatVector, delta };
		return false;
	}

	return true;
}

bool ASteeringAIController::AlignRoll()
{
	SteeringQuat = FQuat::Identity;
	return Align3D(MyUfo->GetActorRotation().Roll, Target->GetActorRotation().Roll, FVector::ForwardVector);
}

bool ASteeringAIController::AlignPitch()
{
	SteeringQuat = FQuat::Identity;
	return Align3D(MyUfo->GetActorRotation().Pitch, Target->GetActorRotation().Pitch, FVector::RightVector);
}

bool ASteeringAIController::AlignYaw()
{
	SteeringQuat = FQuat::Identity;
	return Align3D(MyUfo->GetActorRotation().Yaw, Target->GetActorRotation().Yaw, FVector::UpVector);
}

/*
* Get the Yaw and Pitch to face the target
* Add some roll for "turns", just like planes, but in space...
*/
bool ASteeringAIController::Face3D(const FVector& normalDir)
{
	FVector actorForward = MyUfo->GetActorForwardVector();
	//Stop check
	if (FVector::Coincident(actorForward, normalDir))
	{
		MyUfo->StopAngularMovement();
		SteeringQuat = FQuat::Identity;
		return true;
	}

	//Get a local direction to the target
	FVector localDirectionToTarget = MyUfo->GetActorTransform().InverseTransformPosition(Target->GetActorLocation());

	float yaw;
	float pitch;
	float roll = 0.f;
	UKismetMathLibrary::GetYawPitchFromVector(localDirectionToTarget,yaw, pitch);

	SteeringQuat = FQuat::Identity;
	if (bAddRollOnTurns)
	{
		//Add some rolls on turns, just for fun (negative rollscale work best)
		roll = FMath::DegreesToRadians(yaw * RollScale);
		SteeringQuat *= FQuat(FVector::ForwardVector, roll);
	}
		
	float pitchInput = FMath::DegreesToRadians(pitch);
	float yawInput = FMath::DegreesToRadians(yaw);
	SteeringQuat *= FQuat(FVector::RightVector, pitchInput);
	SteeringQuat *= FQuat(FVector::UpVector, yawInput);

	return false;
}

/*
* Move to get in the radius of target position (forward only, Face3D sets the orientation)
* 
* Returns true when in radius, false otherwise
*/
bool ASteeringAIController::Arrive()
{
	//Set the velocity to get to the target
	FVector direction = Target->GetActorLocation() - MyUfo->GetActorLocation();
	float distance = direction.Size();
	FVector normalDir = direction / distance;

	float targetSpeed = 0.f;
	//Stop check
	if (distance < TargetReachedRadius)
	{
		//Target zone reached
		SteeringVelocity = FVector::ZeroVector;
		MyUfo->StopLinearMovement();
		return true;
	}
	
	Face3D(normalDir);

	float maxSpeed = MyUfo->GetMaxSpeed();
	if (distance > TargetSlowRadius)
	{
		targetSpeed = maxSpeed;
	}
	else 
	{
		targetSpeed = maxSpeed * distance / TargetSlowRadius;
	}

	FVector curForward = MyUfo->GetActorForwardVector();
	//Reduce the speed until we are in the direction of the target (Face3D completes)
	targetSpeed *= FVector::DotProduct(curForward, normalDir);
	
	//Make sure the speed is at least the min, and at most the max
	targetSpeed = FMath::Clamp(targetSpeed, MyUfo->GetMinSpeed(), maxSpeed);

	FVector targetVelocity = curForward * targetSpeed;
	SteeringVelocity = (targetVelocity - MyUfo->GetVelocity()) / TimeToTarget;

	float maxAcceleration = MyUfo->GetMaxAcceleration();
	if (SteeringVelocity.Size() > maxAcceleration)
	{
		SteeringVelocity = SteeringVelocity.GetClampedToMaxSize(maxAcceleration);
	}

	return false;
}


void ASteeringAIController::StopMovement()
{
	NavStatus = ENavStatus::ENS_Stopped;
	SteeringVelocity = FVector::ZeroVector;
	SteeringQuat = FQuat::Identity;
	MyUfo->SetLinearInput(SteeringVelocity);
	MyUfo->SetAngularInput(SteeringQuat);
}


/*
* Blueprint callable to change the target
*/
void ASteeringAIController::SetTarget(ATargetPoint* newTarget)
{
	Target = newTarget;
	bTargetMoved = true;
	StopMovement();	
}

void ASteeringAIController::Tick(float DeltaSeconds)
{
	if (MyUfo != nullptr)
	{
		UpdateState(DeltaSeconds);
		if (NavStatus != ENS_Stopped)
		{
			if (bDebugAll || bDebugRotation || bDebugMovement)
			{
				DebugDisplay();
			}
			MyUfo->SetLinearInput(SteeringVelocity);
			MyUfo->SetAngularInput(SteeringQuat);
		}
	}
}

void ASteeringAIController::DebugDisplay() const
{
	if (MyUfo == nullptr) { return; }

	FVector pawnLoc = MyUfo->GetActorLocation();
	if (bDebugAll || bDebugRotation)
	{
		FRotator rotator = SteeringQuat.Rotator();
		GEngine->AddOnScreenDebugMessage(-1, DebugOnScreenTime, FColor::Orange, FString::Printf(TEXT("Yaw: %.2f | Pitch : %.2f | Roll : %.2f "), rotator.Yaw, rotator.Pitch, rotator.Roll));
		UE_LOG(LogTemp, Warning, TEXT("State: %d | Yaw: %.2f | Pitch : %.2f | Roll : %.2f "), NavStatus.GetValue(), rotator.Yaw, rotator.Pitch, rotator.Roll);

		if (NavStatus == ENS_Moving)
		{
			FVector pawnForward = MyUfo->GetActorForwardVector();
			DrawDebugDirectionalArrow(GetWorld(), pawnLoc, pawnLoc + pawnForward * 200.f, 120.f, FColor::White, false, DebugOnScreenTime, 0, 5.f);
			DrawDebugDirectionalArrow(GetWorld(), pawnLoc, pawnLoc + SteeringQuat.RotateVector(pawnForward) * 200.f, 120.f, FColor::Blue, false, DebugOnScreenTime, 0, 5.f);
		}
	}

	if ((bDebugAll || bDebugMovement) && SteeringVelocity.SizeSquared() > 0.f)
	{
		FVector pawnVelocity = MyUfo->GetVelocity();
		float pawnSpeed = pawnVelocity.Size();
		float steeringSpeed = SteeringVelocity.Size();
		GEngine->AddOnScreenDebugMessage(-1, DebugOnScreenTime, FColor::Orange, FString::Printf(TEXT("Current Speed: %.2f, Desired Speed : %.2f"), pawnSpeed, steeringSpeed));
		DrawDebugDirectionalArrow(GetWorld(), pawnLoc, pawnLoc + SteeringVelocity, 120.f, FColor::Orange, false, DebugOnScreenTime, 0, 5.f);
	}
}