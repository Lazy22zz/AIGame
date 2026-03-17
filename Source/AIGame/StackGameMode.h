// Stack Up! - StackGameMode.h
// All game rules: spawning, placement, scoring, difficulty ramp, camera following.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "StackGameMode.generated.h"

class AStackBlock;
class ACameraActor;

UENUM(BlueprintType)
enum class EStackGameState : uint8
{
	WaitingToStart,
	Playing,
	GameOver
};

UCLASS()
class AIGAME_API AStackGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AStackGameMode();

	// Called by the player controller when Space / LMB is pressed
	void PlaceActiveBlock();

	// Called by the player controller when R is pressed
	void RestartGame();

	// ---- HUD accessors ----
	EStackGameState GetGameState()      const { return GameState; }
	int32           GetScore()          const { return Score; }
	int32           GetBestScore()      const { return BestScore; }
	int32           GetComboCount()     const { return ComboCount; }
	bool            IsPerfectFlashActive() const { return bPerfectFlashActive; }
	float           GetPerfectFlashAlpha() const;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	// ---- Game state ----
	EStackGameState GameState;
	int32  Score;
	int32  BestScore;
	int32  StackCount;  // number of blocks placed (including base)
	int32  ComboCount;  // consecutive perfect placements

	// ---- Block dimensions (shrink as blocks are cut) ----
	float  CurrentWidth;   // X extent of the next block to spawn
	float  CurrentDepth;   // Y extent of the next block to spawn
	static constexpr float BlockH         = 40.f;
	static constexpr float InitialSize    = 300.f;
	static constexpr float MinBlockSize   = 20.f;  // don't cut below this

	// ---- Movement direction alternates each turn ----
	bool  bNextMoveInX;
	float CurrentMoveSpeed;
	static constexpr float SpeedBase      = 180.f;
	static constexpr float SpeedMax       = 580.f;
	static constexpr float SpeedPerBlock  = 18.f;

	// ---- Stack center drift (as blocks are cut off-center) ----
	float  StackCenterX;
	float  StackCenterY;
	float  CurrentTopZ;   // top surface of the highest placed block

	// ---- Actors ----
	UPROPERTY()
	AStackBlock* ActiveBlock;

	UPROPERTY()
	TArray<AStackBlock*> StackedBlocks;

	UPROPERTY()
	ACameraActor* GameCamera;

	// ---- Perfect-flash animation ----
	bool  bPerfectFlashActive;
	float PerfectFlashTimer;
	static constexpr float FlashDuration = 0.7f;

	// ---- Helpers ----
	void SpawnBaseBlock();
	void SpawnNextBlock();
	void SpawnFallingCutPiece(float CX, float CY, float W, float D, float Z,
	                          FLinearColor Color, FVector Impulse);
	void SetupCamera();
	void UpdateCamera();
	void ClearAllBlocks();
	FLinearColor BlockColor(int32 Idx) const;
};
