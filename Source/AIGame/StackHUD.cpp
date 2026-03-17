// Stack Up! - StackHUD.cpp
// All coordinates and sizes are authored at a 1920×1080 reference resolution
// and multiplied by GetUIScale() so they look correct on any screen density.

#include "StackHUD.h"
#include "StackGameMode.h"
#include "Engine/Canvas.h"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

float AStackHUD::GetUIScale() const
{
	if (!Canvas) return 1.f;
	// Scale to the shorter axis so layout never overflows on portrait or ultra-wide.
	const float SX = Canvas->SizeX / 1920.f;
	const float SY = Canvas->SizeY / 1080.f;
	return FMath::Max(FMath::Min(SX, SY), 0.4f); // never go below 0.4 on tiny screens
}

// ---------------------------------------------------------------------------
// Main draw dispatch
// ---------------------------------------------------------------------------

void AStackHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas) return;

	AStackGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AStackGameMode>() : nullptr;
	if (!GM) return;

	switch (GM->GetGameState())
	{
	case EStackGameState::WaitingToStart:
		DrawWaitingUI();
		break;

	case EStackGameState::Playing:
		DrawPlayingUI(
			GM->GetScore(),
			GM->GetComboCount(),
			GM->IsPerfectFlashActive(),
			GM->GetPerfectFlashAlpha()
		);
		break;

	case EStackGameState::GameOver:
		DrawGameOverUI(GM->GetScore(), GM->GetBestScore());
		break;
	}
}

// ---------------------------------------------------------------------------
// Playing state
// ---------------------------------------------------------------------------

void AStackHUD::DrawPlayingUI(int32 Score, int32 Combo, bool bShowPerfect, float PerfectAlpha)
{
	const float S  = GetUIScale();
	const float W  = Canvas->SizeX;
	const float H  = Canvas->SizeY;

	// --- Score panel (top-left) ---
	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.55f), 12.f*S, 12.f*S, 230.f*S, 56.f*S);
	DrawText(FString::Printf(TEXT("SCORE  %d"), Score),
		FColor(255, 235, 80), 22.f*S, 22.f*S, nullptr, 1.7f*S);

	// --- Combo badge (top-right) ---
	if (Combo > 1)
	{
		const FString Str = FString::Printf(TEXT("COMBO  x%d"), Combo);
		DrawRect(FLinearColor(0.9f, 0.4f, 0.f, 0.65f), W - 240.f*S, 12.f*S, 228.f*S, 56.f*S);
		DrawText(Str, FColor(255, 255, 255), W - 228.f*S, 22.f*S, nullptr, 1.7f*S);
	}

	// --- "PERFECT!" flash (center-top) ---
	if (bShowPerfect && PerfectAlpha > 0.f)
	{
		const uint8  Alpha    = (uint8)(FMath::Clamp(PerfectAlpha, 0.f, 1.f) * 255.f);
		const FColor PerfCol(80, 255, 120, Alpha);
		DrawText(TEXT("PERFECT!"), PerfCol,
			W * 0.5f - 110.f*S, H * 0.5f - 120.f*S, nullptr, 3.5f*S);
	}

	// --- Controls hint (bottom-center) — shows both touch and keyboard ---
	const FString HintStr = TEXT("TAP / SPACE  to place     R  to restart");
	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.45f),
		W * 0.5f - 280.f*S, H - 54.f*S, 560.f*S, 42.f*S);
	DrawText(HintStr, FColor(200, 200, 200),
		W * 0.5f - 268.f*S, H - 44.f*S, nullptr, 1.2f*S);
}

// ---------------------------------------------------------------------------
// Game-over overlay
// ---------------------------------------------------------------------------

void AStackHUD::DrawGameOverUI(int32 Score, int32 BestScore)
{
	const float S  = GetUIScale();
	const float W  = Canvas->SizeX;
	const float H  = Canvas->SizeY;

	// Full-screen dark overlay
	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.72f), 0.f, 0.f, W, H);

	// Centre panel
	const float PW = 480.f*S;
	const float PH = 260.f*S;
	const float PX = (W - PW) * 0.5f;
	const float PY = (H - PH) * 0.5f;
	DrawRect(FLinearColor(0.05f, 0.05f, 0.08f, 0.93f), PX, PY, PW, PH);
	// Accent bar at top of panel
	DrawRect(FLinearColor(0.85f, 0.15f, 0.15f, 1.f), PX, PY, PW, 7.f*S);

	DrawText(TEXT("GAME  OVER"),
		FColor(240, 60, 60), PX + 52.f*S, PY + 22.f*S, nullptr, 2.8f*S);

	DrawText(FString::Printf(TEXT("Score  :  %d"), Score),
		FColor(255, 235, 80), PX + 64.f*S, PY + 110.f*S, nullptr, 1.9f*S);

	DrawText(FString::Printf(TEXT("Best   :  %d"), BestScore),
		FColor(160, 220, 255), PX + 64.f*S, PY + 152.f*S, nullptr, 1.9f*S);

	// Tap-to-restart hint — prominently shown for mobile users
	DrawText(TEXT("TAP  or  press  R  to  restart"),
		FColor(210, 210, 210), PX + 38.f*S, PY + 218.f*S, nullptr, 1.25f*S);
}

// ---------------------------------------------------------------------------
// Waiting / start screen
// ---------------------------------------------------------------------------

void AStackHUD::DrawWaitingUI()
{
	const float S = GetUIScale();
	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;

	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.55f),
		W * 0.5f - 220.f*S, H * 0.5f - 44.f*S, 440.f*S, 88.f*S);
	DrawText(TEXT("TAP  /  SPACE  to begin!"),
		FColor(255, 235, 80), W * 0.5f - 200.f*S, H * 0.5f - 22.f*S, nullptr, 2.1f*S);
}
