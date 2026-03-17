// Stack Up! - StackHUD.cpp

#include "StackHUD.h"
#include "StackGameMode.h"
#include "Engine/Canvas.h"

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

void AStackHUD::DrawPlayingUI(int32 Score, int32 Combo, bool bShowPerfect, float PerfectAlpha)
{
	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;

	// --- Score panel (top-left) ---
	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.55f), 12.f, 12.f, 210.f, 50.f);
	DrawText(FString::Printf(TEXT("SCORE  %d"), Score),
		FColor(255, 235, 80), 22.f, 22.f, nullptr, 1.6f);

	// --- Combo indicator (top-right) ---
	if (Combo > 1)
	{
		FString ComboStr = FString::Printf(TEXT("COMBO  x%d"), Combo);
		DrawRect(FLinearColor(0.9f, 0.4f, 0.f, 0.6f), W - 220.f, 12.f, 210.f, 50.f);
		DrawText(ComboStr, FColor(255, 255, 255), W - 210.f, 22.f, nullptr, 1.6f);
	}

	// --- "PERFECT!" flash (center) ---
	if (bShowPerfect && PerfectAlpha > 0.f)
	{
		const uint8 Alpha = (uint8)(FMath::Clamp(PerfectAlpha, 0.f, 1.f) * 255.f);
		const FColor PerfColor(80, 255, 120, Alpha);
		DrawText(TEXT("PERFECT!"), PerfColor, W / 2.f - 100.f, H / 2.f - 100.f, nullptr, 3.2f);
	}

	// --- Controls hint (bottom-center) ---
	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.45f), W / 2.f - 160.f, H - 48.f, 320.f, 36.f);
	DrawText(TEXT("SPACE: Place   |   R: Restart"),
		FColor(200, 200, 200), W / 2.f - 148.f, H - 40.f, nullptr, 1.1f);
}

void AStackHUD::DrawGameOverUI(int32 Score, int32 BestScore)
{
	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;

	// Dark overlay
	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.72f), 0.f, 0.f, W, H);

	// Panel
	const float PW = 440.f;
	const float PH = 220.f;
	const float PX = (W - PW) * 0.5f;
	const float PY = (H - PH) * 0.5f;
	DrawRect(FLinearColor(0.05f, 0.05f, 0.08f, 0.92f), PX, PY, PW, PH);
	DrawRect(FLinearColor(0.85f, 0.15f, 0.15f, 1.f), PX, PY, PW, 6.f); // top accent bar

	// "GAME OVER"
	DrawText(TEXT("GAME  OVER"), FColor(240, 60, 60), PX + 50.f, PY + 22.f, nullptr, 2.6f);

	// Score
	DrawText(FString::Printf(TEXT("Score  :  %d"), Score),
		FColor(255, 235, 80), PX + 60.f, PY + 100.f, nullptr, 1.8f);

	// Best
	DrawText(FString::Printf(TEXT("Best   :  %d"), BestScore),
		FColor(160, 220, 255), PX + 60.f, PY + 140.f, nullptr, 1.8f);

	// Restart hint
	DrawText(TEXT("Press  R  to  restart"),
		FColor(180, 180, 180), PX + 80.f, PY + 186.f, nullptr, 1.2f);
}

void AStackHUD::DrawWaitingUI()
{
	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;

	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.55f), W / 2.f - 200.f, H / 2.f - 40.f, 400.f, 80.f);
	DrawText(TEXT("Press SPACE to begin!"),
		FColor(255, 235, 80), W / 2.f - 185.f, H / 2.f - 20.f, nullptr, 2.0f);
}
