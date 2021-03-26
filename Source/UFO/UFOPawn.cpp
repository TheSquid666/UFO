// Copyright Epic Games, Inc. All Rights Reserved.

#include "UFOPawn.h"
#include "Engine/TargetPoint.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "DrawDebugHelpers.h"

AUFOPawn::AUFOPawn()
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> PlaneMesh;
		FConstructorStatics()
			: PlaneMesh(TEXT("/Game/Flying/Meshes/UFO.UFO"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	// Create static mesh component
	PlaneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMesh0"));
	PlaneMesh->SetStaticMesh(ConstructorStatics.PlaneMesh.Get());	// Set static mesh
	RootComponent = PlaneMesh;

	// Create a spring arm component
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->SetupAttachment(RootComponent);	// Attach SpringArm to RootComponent
	SpringArm->TargetArmLength = 160.0f; // The camera follows at this distance behind the character	
	SpringArm->SocketOffset = FVector(0.f,0.f,60.f);
	SpringArm->bEnableCameraLag = false;	// Do not allow camera to lag
	SpringArm->CameraLagSpeed = 15.f;

	// Create camera component 
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);	// Attach the camera
	Camera->bUsePawnControlRotation = false; // Don't rotate camera with controller

	// Set handling parameters
	//Acceleration = 500.f;
	//TurnSpeed = 50.f;
	//MaxSpeed = 4000.f;
	//MinSpeed = 500.f;
	linearForce = { 100.0f, 100.0f, 100.0f };
	angularForce = { 100.0f, 100.0f, 100.0f };
	Stop();
}

void AUFOPawn::SetPhysicsInput(FVector linearInput, FVector angularInput)
{
	appliedLinearForce = linearInput * linearForce;
	appliedAngularForce = angularInput * angularForce;
}

void AUFOPawn::FixedUpdate()
{
	if (PlaneMesh != nullptr)
	{
		PlaneMesh->AddImpulse(appliedLinearForce * forceMultiplier);
		PlaneMesh->AddRadialImpulse(GetActorLocation(), appliedAngularForce * forceMultiplier, 50.f);
	}
}

void AUFOPawn::Tick(float DeltaSeconds)
{
	if (EnableAi) {
		UpdateState(DeltaSeconds);
	}
	else {
		SetPhysicsInput(FVector{ strafe, 0.0f, throttle } *DeltaSeconds, FVector{ pitch, yaw, roll } *DeltaSeconds);
	}

	//TODO implement sub for Fixed update
	FixedUpdate();
	//const FVector LocalMove = FVector(CurrentForwardSpeed * DeltaSeconds, 0.f, 0.f);

	// Move plan forwards (with sweep so we stop when we collide with things)
	//AddActorLocalOffset(LocalMove, true);

	// Calculate change in rotation this frame
	//FRotator DeltaRotation(0,0,0);
	//DeltaRotation.Pitch = CurrentPitchSpeed * DeltaSeconds;
	//DeltaRotation.Yaw = CurrentYawSpeed * DeltaSeconds;
	//DeltaRotation.Roll = CurrentRollSpeed * DeltaSeconds;

	// Rotate plane
	//AddActorLocalRotation(DeltaRotation);



	// Call any parent class Tick implementation
	Super::Tick(DeltaSeconds);
}



void AUFOPawn::Stop()
{
	NavStatus = Status::Stopped;
	appliedLinearForce = FVector::ZeroVector;
	appliedAngularForce = FVector::ZeroVector;
	//CurrentForwardSpeed = 0.f;
	//CurrentPitchSpeed = 0.f;
	//CurrentYawSpeed = 0.f;
	//CurrentRollSpeed = 0.f;
}



void AUFOPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
    // Check if PlayerInputComponent is valid (not NULL)
	check(PlayerInputComponent);

	// Bind our control axis' to callback functions
	PlayerInputComponent->BindAxis("Thrust", this, &AUFOPawn::ThrustInput);
	PlayerInputComponent->BindAxis("MoveUp", this, &AUFOPawn::MoveUpInput);
	PlayerInputComponent->BindAxis("MoveRight", this, &AUFOPawn::MoveRightInput);
}


void AUFOPawn::ThrustInput(float Val)
{
	// Is there any input?
	bool bHasInput = !FMath::IsNearlyEqual(Val, 0.f);

	if (LeftTruster != nullptr)
	{
		if (bHasInput)
		{
			LeftTruster->Activate();
		}
		else
		{
			LeftTruster->Deactivate();
		}
	}

	if (RightTruster != nullptr)
	{
		if (bHasInput)
		{
			RightTruster->Activate();
		}
		else
		{
			RightTruster->Deactivate();
		}
	}
	throttle = Val * THROTTLE_SPEED;

	//// If input is not held down, reduce speed
	//float CurrentAcc = bHasInput ? (Val * Acceleration) : (-0.1f * Acceleration);
	//// Calculate new speed
	//float NewForwardSpeed = CurrentForwardSpeed + (GetWorld()->GetDeltaSeconds() * CurrentAcc);
	//// Clamp between MinSpeed and MaxSpeed
	//CurrentForwardSpeed = FMath::Clamp(NewForwardSpeed, MinSpeed, MaxSpeed);
}

void AUFOPawn::MoveUpInput(float Val)
{
	pitch = Val;

	//// Target pitch speed is based in input
	//float TargetPitchSpeed = (Val * TurnSpeed * -1.f);

	//// When steering, we decrease pitch slightly
	//TargetPitchSpeed += (FMath::Abs(CurrentYawSpeed) * -0.2f);

	//// Smoothly interpolate to target pitch speed
	//CurrentPitchSpeed = FMath::FInterpTo(CurrentPitchSpeed, TargetPitchSpeed, GetWorld()->GetDeltaSeconds(), 2.f);
}

void AUFOPawn::MoveRightInput(float Val)
{
	yaw = Val;

	//// Target yaw speed is based on input
	//float TargetYawSpeed = (Val * TurnSpeed);

	//// Smoothly interpolate to target yaw speed
	//CurrentYawSpeed = FMath::FInterpTo(CurrentYawSpeed, TargetYawSpeed, GetWorld()->GetDeltaSeconds(), 2.f);

	//// Is there any left/right input?
	//const bool bIsTurning = FMath::Abs(Val) > 0.2f;

	//// If turning, yaw value is used to influence roll
	//// If not turning, roll to reverse current roll value.
	//float TargetRollSpeed = bIsTurning ? (CurrentYawSpeed * 0.5f) : (GetActorRotation().Roll * -2.f);

	//// Smoothly interpolate roll speed
	//CurrentRollSpeed = FMath::FInterpTo(CurrentRollSpeed, TargetRollSpeed, GetWorld()->GetDeltaSeconds(), 2.f);
}

bool AUFOPawn::UpdateRotation(float DeltaTime, FVector Direction)
{
	//FRotator NewControlRotation = Direction.Rotation();
	//float diffBetweenForward = FVector::DotProduct(Direction, GetActorForwardVector());

	//if (FMath::IsNearlyEqual(diffBetweenForward, 0.f))
	//{
	//	//Nothing to do
	//	return true;
	//}

	//NewControlRotation.Yaw = FRotator::ClampAxis(NewControlRotation.Yaw);
	//NewControlRotation.Pitch = FRotator::ClampAxis(NewControlRotation.Pitch);

	//MoveRightInput(NewControlRotation.Yaw / 360.f);
	//MoveUpInput(NewControlRotation.Pitch / -360.f);

	//return false;
}

bool AUFOPawn::UpdateMovement(float DeltaTime)
{
	//FVector targetPos = MoveTarget->GetActorLocation();
	//FVector curPos = GetActorLocation();
	//float dot = FVector::DotProduct(GetActorForwardVector(), targetPos - curPos);
	//if (FMath::IsNearlyEqual(dot, 0.f))
	//{
	//	//Nothing to do
	//	return true;
	//}

	//ThrustInput(dot);
	//return false;
}

bool AUFOPawn::IsTargetReached()
{
	FVector targetPos = MoveTarget->GetActorLocation();
	FVector curPos = GetActorLocation();
	float sqrDistToTarget = FVector::DistSquared(curPos, targetPos);	
	return sqrDistToTarget <= TargetReachedDist * TargetReachedDist;
}


void AUFOPawn::UpdateState(float DeltaTime)
{
	if (MoveTarget == nullptr)
	{
		Stop();
		return;
	}

	FVector targetPos = MoveTarget->GetActorLocation();
	FVector curPos = GetActorLocation();

	switch (NavStatus)
	{
	case Status::Stopped:
		//We have a target, face it
		NavStatus = Status::FaceTarget;
		[[fallthrough]];
	case Status::FaceTarget:
		{
			FVector Direction = targetPos - curPos;
			if (DebugAi)
			{
				DrawDebugDirectionalArrow(GetWorld(), curPos, GetActorForwardVector() * 200 + curPos, 120.f, FColor::Yellow, true, 0.03f, 0, 5.f);
				DrawDebugDirectionalArrow(GetWorld(), curPos, Direction.GetSafeNormal() * 500 + curPos, 120.f, FColor::Magenta, true, 0.03f, 0, 5.f);
				DrawDebugDirectionalArrow(GetWorld(), curPos, targetPos, 120.f, FColor::Blue, true, 0.03f, 0, 5.f);
			}

			if (UpdateRotation(DeltaTime, Direction))
			{
				NavStatus = Status::Moving;
			}
		}
		break;
	case Status::Moving:
	{
		if (IsTargetReached())
		{
			//At target, orient to match the expected orientation
			NavStatus = Status::OrientAtTarget;
		}
		else
		{
			if (UpdateMovement(DeltaTime))
			{
				NavStatus = Status::OrientAtTarget;
			}
		}
	}
		break;
	case Status::OrientAtTarget:
	{
		FVector Direction = MoveTarget->GetActorForwardVector();
		if (DebugAi)
		{
			DrawDebugDirectionalArrow(GetWorld(), curPos, GetActorForwardVector() * 500 + curPos, 120.f, FColor::Green, true, 0.03f, 0, 5.f);
			DrawDebugDirectionalArrow(GetWorld(), curPos, Direction.GetSafeNormal() * 500 + curPos, 120.f, FColor::Red, true, 0.03f, 0, 5.f);
		}
		if (UpdateRotation(DeltaTime, Direction))
		{
			Stop();
		}
	}
		break;
	}
}
