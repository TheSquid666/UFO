// Fill out your copyright notice in the Description page of Project Settings.


#include "SteeringAIController.h"
#include "Engine/TargetPoint.h"
#include "UFOPawn.h"
#include "DrawDebugHelpers.h"

ASteeringAIController::ASteeringAIController() 
	: MyUfo{ nullptr }
	, SteeringVelocity{FVector::ZeroVector}
	, SteeringQuat{ FQuat::Identity }
	, bTargetMoved{ true }
	, bDebugAll{true}
	, bDebugRotation{false}
	, bDebugMovement{false}
	, DebugOnScreenTime{0.003f}
	, Target{nullptr}
	, TargetReachedRadius{5.f}
	, TargetSlowRadius{500.f}
	, TimeToTarget{0.1f}
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

void ASteeringAIController::UpdateState(float DeltaTime)
{
	if (Target == nullptr || MyUfo == nullptr)
	{
		StopMovement();
		return;
	}

	FVector targetPos = Target->GetActorLocation();
	FVector curPos = MyUfo->GetActorLocation();

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
				NavStatus = ENavStatus::ENS_Aligning;
			}
		}
		break;
	case ENavStatus::ENS_Aligning:
		{
			//Align with the desired orientation
			if (Align3D())
			{
				NavStatus = ENavStatus::ENS_Stopped;
			}
		}
		break;
	}
}



/*
* Align one of the yaw, pitch roll at a time
*/
bool ASteeringAIController::Align3D()
{
	FRotator desiredOrientation = Target->GetActorRotation();
	FRotator actualOrientation = MyUfo->GetActorRotation();
	if (!FMath::IsNearlyEqual(actualOrientation.Yaw, desiredOrientation.Yaw, 1.f))
	{
		float delta = desiredOrientation.Yaw - actualOrientation.Yaw;	
		SteeringQuat = FQuat{ FRotator{ 0.f, delta, 0.f } };
	}
	else if (!FMath::IsNearlyEqual(actualOrientation.Pitch, desiredOrientation.Pitch, 1.f))
	{
		float delta = desiredOrientation.Pitch - actualOrientation.Pitch;
		SteeringQuat = FQuat{ FRotator{ delta, 0.f, 0.f } };
	}
	else if (!FMath::IsNearlyEqual(actualOrientation.Roll, desiredOrientation.Roll, 1.f))
	{
		float delta = desiredOrientation.Roll - actualOrientation.Roll;		
		SteeringQuat = FQuat{ FRotator{ 0.f, 0.f, delta } };
	}
	else
	{
		MyUfo->StopAngularMovement();
		SteeringQuat = FQuat::Identity;
		return true;
	}

	return false;
}

bool ASteeringAIController::Face3D(const FVector& normalDir)
{
	FVector actorForward = MyUfo->GetActorForwardVector();
	if (FVector::Coincident(actorForward, normalDir))
	{
		MyUfo->StopAngularMovement();
		SteeringQuat = FQuat::Identity;
		return true;
	}

	SteeringQuat = FQuat::FindBetweenNormals(actorForward, normalDir);

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
	
	if (distance < TargetReachedRadius)
	{
		//Target zone reached
		SteeringVelocity = FVector::ZeroVector;
		MyUfo->StopLinearMovement();
		return true;
	}

	//Turn to where you are going
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
	targetSpeed *= FVector::DotProduct(curForward, normalDir);
	
	//Make sure the speed is a least the min, and at most the max
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
		FVector pawnForward = MyUfo->GetActorForwardVector();
		DrawDebugDirectionalArrow(GetWorld(), pawnLoc, pawnLoc + pawnForward *200.f, 120.f, FColor::White, false, DebugOnScreenTime, 0, 5.f);
		DrawDebugDirectionalArrow(GetWorld(), pawnLoc, pawnLoc + SteeringQuat.RotateVector(pawnForward) * 200.f, 120.f, FColor::Blue, false, DebugOnScreenTime, 0, 5.f);
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