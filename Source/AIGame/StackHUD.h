// Stack Up! - StackHUD.h
// Canvas-based HUD: score, combo, "PERFECT!" flash, game-over overlay, controls hint.

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
	void DrawPlayingUI(int32 Score, int32 Combo, bool bShowPerfect, float PerfectAlpha);
	void DrawGameOverUI(int32 Score, int32 BestScore);
	void DrawWaitingUI();
};
