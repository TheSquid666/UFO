// Copyright Jonathan Paturel, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Math/Vector.h"
#include "Math/Rotator.h"
#include "SteeringAIController.generated.h"

UENUM(Meta = (Bitflags))
enum ENavStatus
{
	ENS_Stopped            UMETA(DisplayName = "Stopped"),	
	ENS_Moving             UMETA(DisplayName = "Moving"),
	ENS_Aligning_yaw       UMETA(DisplayName = "Aligning Yaw"),
	ENS_Aligning_pitch     UMETA(DisplayName = "Aligning Pitch"),
	ENS_Aligning_roll      UMETA(DisplayName = "Aligning Roll"),
};

/**
 * Space Flight Basic Steering Controller
 * Allow for more steering functions
 * 
 * Physics and gravity can be enabled, steering can handle it (untested)
 * If obstacles are added, add pathfinding (Voronoid or 3D nav plugin) and FollowPath and Avoidance behaviours.
 */
UCLASS()
class UFO_API ASteeringAIController : public AAIController
{
	GENERATED_BODY()

private:
	class AUFOPawn* MyUfo;
	FVector SteeringVelocity;
	FQuat SteeringQuat;
	bool bTargetMoved;

public:
	/*Editable*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		bool bDebugAll;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		bool bDebugRotation;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		bool bDebugMovement;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		float DebugOnScreenTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
		class ATargetPoint* Target;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
		float TargetAngleReachedTolerance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
		float TargetReachedRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
		float TargetSlowRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
		float TimeToTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Roll)
		float RollScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Roll)
		bool bAddRollOnTurns;

	/*Read Only*/
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = AI, Meta = (Bitmask, BitmaskEnum = "ENavStatus"))
		TEnumAsByte<ENavStatus> NavStatus;

protected:
	void UpdateState(float DeltaTime);
	bool Align3D(float currentVal, float expectedVal, const FVector& quatVector);
	bool AlignYaw();
	bool AlignPitch();
	bool AlignRoll();
	
	bool Face3D(const FVector& direction);
	bool Arrive();
	void GetFakeOrientation(const FVector& direction);
	void OnPossess(APawn* InPawn) override;
	void OnUnPossess() override;
	void DebugDisplay() const;

public:
	ASteeringAIController();
	~ASteeringAIController() = default;
	void StopMovement() override;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetTarget(ATargetPoint* newTarget);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void Start() { bTargetMoved = true; }

	void Tick(float DeltaSeconds) override;
};
