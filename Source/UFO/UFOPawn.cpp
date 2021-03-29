// Copyright Epic Games, Inc. All Rights Reserved.

#include "UFOPawn.h"

#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "DrawDebugHelpers.h"


AUFOPawn::AUFOPawn() :
	Velocity{ FVector::ZeroVector }, LinearInput{ FVector::ZeroVector }, AngularInput{ FQuat::Identity }, 
	MinSpeed{5.f}, MaxSpeed{ 4000.f }, MaxAcceleration{ 500.f }, MaxRollSpeed{ 10.f }, MaxYawSpeed{ 10.f }, MaxPitchSpeed{ 10.f }
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
	
}

/*
* Enable the Trusters particles when LinearInput is there
*/
void AUFOPawn::UpdateThrustParticles()
{
	bool bHasThrust = !(LinearInput.IsNearlyZero());
	if (LeftTruster != nullptr)
	{
		if (bHasThrust)
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
		if (bHasThrust)
		{
			RightTruster->Activate();
		}
		else
		{
			RightTruster->Deactivate();
		}
	}

	//Todo move the rotation thruster to an angle to make the turn believable
}



void AUFOPawn::StopAllMovement()
{
	StopAngularMovement();
	StopLinearMovement();
}


void AUFOPawn::Tick(float DeltaSeconds)
{
	/*Kinematic updates*/	
	//Linear
	if (!LinearInput.IsNearlyZero())
	{
		AddActorWorldOffset(Velocity * DeltaSeconds);
		Velocity += LinearInput * DeltaSeconds;
		Velocity = Velocity.GetClampedToMaxSize(MaxSpeed);
	}

	//Angular
	//Get Pitch Yaw Roll Input to limit to speed
	if (!AngularInput.IsIdentity())
	{
		FRotator angularRotator = AngularInput.Rotator();

		//Limit the input values on MaxValues and factor in the deltatime
		float rollInput = FMath::DegreesToRadians(FMath::Clamp(angularRotator.Roll * DeltaSeconds, -MaxRollSpeed, MaxRollSpeed)) ;
		float pitchInput = FMath::DegreesToRadians(FMath::Clamp(angularRotator.Pitch * DeltaSeconds, -MaxPitchSpeed, MaxPitchSpeed));
		float yawInput = FMath::DegreesToRadians(FMath::Clamp(angularRotator.Yaw * DeltaSeconds, -MaxYawSpeed, MaxYawSpeed));
		FQuat delta = FQuat(FVector::ForwardVector, rollInput);
		delta *= FQuat(FVector::RightVector, pitchInput);
		delta *= FQuat(FVector::UpVector, yawInput);

		AddActorLocalRotation(delta);
	}

	UpdateThrustParticles();

	Super::Tick(DeltaSeconds);
}



