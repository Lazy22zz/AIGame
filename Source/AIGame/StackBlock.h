// Stack Up! - StackBlock.h
// A single block in the stacking game. Can be in one of three states:
// moving (sliding back and forth), placed (static on the stack), or falling (cut-off piece).

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StackBlock.generated.h"

UCLASS()
class AIGAME_API AStackBlock : public AActor
{
	GENERATED_BODY()

public:
	AStackBlock();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	UStaticMeshComponent* BlockMesh;

	// Logical dimensions (world units / cm)
	float BlockWidth;  // Extent along X
	float BlockDepth;  // Extent along Y
	static constexpr float BlockHeight = 40.f; // Fixed height for all blocks

	// Movement state
	bool bIsMoving;
	bool bMoveAlongX; // true = oscillates along X axis, false = along Y axis
	float MoveSpeed;
	float MoveRange;  // Half-amplitude of oscillation
	FVector MoveOrigin;
	int32 MoveDir;    // +1 or -1

	// Falling state (for cut-off pieces)
	bool bIsFalling;
	FVector FallVelocity;

	/** Set block dimensions, position and color. Safe to call multiple times. */
	void InitBlock(float InWidth, float InDepth, FVector InPosition, FLinearColor InColor);

	/** Begin oscillating in the given axis. */
	void StartMoving(bool bInX, float InSpeed, float InRange, FVector InOrigin);

	/** Freeze in current position. */
	void StopMoving();

	/** Begin falling (cut-off piece animation). */
	void StartFalling(FVector InVelocity);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	UMaterial* BaseMaterial;
};
