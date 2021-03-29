// Copyright Jonathan Paturel, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "UFOPawn.generated.h"

/*
* UFO Actor (Simple and Quick)
* Contain the Movement/Physics in this class for space flight
* Using Kinematic without Impluses
* TODO: If this is to be used in Unreal, just use a MovementComponent instead
*/
UCLASS(Config=Game)
class AUFOPawn : public APawn
{
	GENERATED_BODY()

	/** StaticMesh component that will be the visuals for our flying pawn */
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* PlaneMesh;

	/** Spring arm that will offset the camera */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArm;

	/** Camera component that will be our viewpoint */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;

	/*Implement the movement component in the actor since it is part of the requirements*/
	FVector Velocity;
	//FQuat Rotation;

	FVector LinearInput;
	FQuat AngularInput;

protected:
	void UpdateThrustParticles();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
		class ATargetPoint* Target;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ParticleSystem)
		class UParticleSystemComponent* LeftTruster;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ParticleSystem)
		class UParticleSystemComponent* RightTruster;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ParticleSystem)
		class UParticleSystemComponent* RotationTruster;


public:
	void SetLinearInput(FVector& newLinearInput) { LinearInput = newLinearInput; }
	void SetAngularInput(FQuat& newAngularInput) { AngularInput = newAngularInput; }

	AUFOPawn();

	// Begin AActor overrides
	virtual void Tick(float DeltaSeconds) override;
	// End AActor overrides
	void StopAllMovement();

	FORCEINLINE void StopAngularMovement()
	{
		AngularInput = FQuat::Identity;
	}

	FORCEINLINE void StopLinearMovement()
	{
		Velocity = LinearInput = FVector::ZeroVector;
	}


private:

	UPROPERTY(Category = Movement, EditAnywhere)
		float MinSpeed;

	/** Max forward speed */
	UPROPERTY(Category = Movement, EditAnywhere)
		float MaxSpeed;

	UPROPERTY(Category = Movement, EditAnywhere)
		float MaxAcceleration;

	/** How quickly pawn can steer */
	UPROPERTY(Category = Movement, EditAnywhere)
	float MaxRollSpeed;

	UPROPERTY(Category = Movement, EditAnywhere)
	float MaxYawSpeed;

	UPROPERTY(Category = Movement, EditAnywhere)
	float MaxPitchSpeed;

public:
	void RandomlyPlaceTarget();

	/** Returns PlaneMesh subobject **/
	FORCEINLINE class UStaticMeshComponent* GetPlaneMesh() const { return PlaneMesh; }
	/** Returns SpringArm subobject **/
	FORCEINLINE class USpringArmComponent* GetSpringArm() const { return SpringArm; }
	/** Returns Camera subobject **/
	FORCEINLINE class UCameraComponent* GetCamera() const { return Camera; }

	FORCEINLINE class UParticleSystemComponent* GetRightTruster() const { return RightTruster; }
	FORCEINLINE class UParticleSystemComponent* GetLeftTruster() const { return LeftTruster; }
	FORCEINLINE class UParticleSystemComponent* GetRotationTruster() const { return RotationTruster; }

	FORCEINLINE float GetMaxRollSpeed() const { return MaxRollSpeed; }
	FORCEINLINE float GetMaxYawSpeed() const { return MaxYawSpeed; }
	FORCEINLINE float GetMaxPitchSpeed() const { return MaxPitchSpeed; }
	FORCEINLINE float GetMaxSpeed() const { return MaxSpeed; }
	FORCEINLINE float GetMinSpeed() const { return MinSpeed; }
	FORCEINLINE float GetMaxAcceleration() const {return MaxAcceleration;}

	FORCEINLINE FVector GetVelocity() const { return Velocity; }
	FORCEINLINE ATargetPoint* GetTarget() const { return Target; }
};
