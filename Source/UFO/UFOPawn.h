// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "UFOPawn.generated.h"

UENUM()
enum Status
{
	Stopped            UMETA(DisplayName = "Stopped"),
	FaceTarget         UMETA(DisplayName = "FaceTarget"),
	Moving             UMETA(DisplayName = "Moving"),
	OrientAtTarget     UMETA(DisplayName = "OrientAtTarget"),
};


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

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
		bool EnableAi;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
		bool DebugAi;

	UPROPERTY(VisibleAnywhere, Category = AI)
		TEnumAsByte<Status> NavStatus;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AI)
		bool TargetReached;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
		class ATargetPoint* MoveTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
		float TargetReachedDist;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ParticleSystem)
		class UParticleSystemComponent* LeftTruster;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ParticleSystem)
		class UParticleSystemComponent* RightTruster;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ParticleSystem)
		class UParticleSystemComponent* RotationTruster;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Navigation)
	float pitch;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Navigation)
	float yaw;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Navigation)
	float roll;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Navigation)
	float strafe;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Navigation)
	float throttle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Navigation)
	float THROTTLE_SPEED = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Navigation)
	FVector linearForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Navigation)
	FVector angularForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Navigation)
	float reverseMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Navigation)
	float forceMultiplier = 100.0f;

public:
	AUFOPawn();

	// Begin AActor overrides
	virtual void Tick(float DeltaSeconds) override;
	// End AActor overrides

	bool UpdateRotation(float DeltaTime, FVector Direction);
	bool UpdateMovement(float DeltaTime);

	void Stop();
	
	bool IsTargetReached();


protected:

	// Begin APawn overrides
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override; // Allows binding actions/axes to functions
	// End APawn overrides

	/** Bound to the thrust axis */
	void ThrustInput(float Val);
	
	/** Bound to the vertical axis */
	void MoveUpInput(float Val);

	/** Bound to the horizontal axis */
	void MoveRightInput(float Val);

	//AI Navigation
	void UpdateState(float DeltaTime);

	void FixedUpdate();
	void SetPhysicsInput(FVector linearInput, FVector angularInput);
private:

	///** How quickly forward speed changes */
	//UPROPERTY(Category=Plane, EditAnywhere)
	//float Acceleration;

	///** How quickly pawn can steer */
	//UPROPERTY(Category=Plane, EditAnywhere)
	//float TurnSpeed;

	///** Max forward speed */
	//UPROPERTY(Category = Pitch, EditAnywhere)
	//float MaxSpeed;

	///** Min forward speed */
	//UPROPERTY(Category=Yaw, EditAnywhere)
	//float MinSpeed;

	///** Current forward speed */
	//float CurrentForwardSpeed;

	///** Current yaw speed */
	//float CurrentYawSpeed;

	///** Current pitch speed */
	//float CurrentPitchSpeed;

	///** Current roll speed */
	//float CurrentRollSpeed;
	FVector appliedLinearForce;
	FVector appliedAngularForce;

public:
	/** Returns PlaneMesh subobject **/
	FORCEINLINE class UStaticMeshComponent* GetPlaneMesh() const { return PlaneMesh; }
	/** Returns SpringArm subobject **/
	FORCEINLINE class USpringArmComponent* GetSpringArm() const { return SpringArm; }
	/** Returns Camera subobject **/
	FORCEINLINE class UCameraComponent* GetCamera() const { return Camera; }

	FORCEINLINE class UParticleSystemComponent* GetRightTruster() const { return RightTruster; }
	FORCEINLINE class UParticleSystemComponent* GetLeftTruster() const { return LeftTruster; }
	FORCEINLINE class UParticleSystemComponent* GetRotationTruster() const { return RotationTruster; }
};
