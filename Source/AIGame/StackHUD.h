// Stack Up! - StackHUD.h
// Canvas-based HUD: score, combo, "PERFECT!" flash, game-over overlay, controls hint.
// All element sizes are defined at a 1920x1080 reference and scaled to the actual
// canvas size — this ensures correct proportions on both 4K phones and 1080p desktop.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "StackHUD.generated.h"

UCLASS()
class AIGAME_API AStackHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	/** Uniform scale factor relative to a 1920×1080 reference resolution. */
	float GetUIScale() const;

	void DrawPlayingUI(int32 Score, int32 Combo, bool bShowPerfect, float PerfectAlpha);
	void DrawGameOverUI(int32 Score, int32 BestScore);
	void DrawWaitingUI();
};
