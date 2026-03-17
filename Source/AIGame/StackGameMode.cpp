// Stack Up! - StackGameMode.cpp

#include "StackGameMode.h"
#include "StackBlock.h"
#include "StackPlayerController.h"
#include "StackHUD.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "EngineUtils.h"

AStackGameMode::AStackGameMode()
{
	PrimaryActorTick.bCanEverTick = true;

	PlayerControllerClass = AStackPlayerController::StaticClass();
	HUDClass              = AStackHUD::StaticClass();
	DefaultPawnClass      = nullptr; // Camera is managed directly; no pawn needed
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void AStackGameMode::BeginPlay()
{
	Super::BeginPlay();

	Score           = 0;
	BestScore       = 0;
	StackCount      = 0;
	ComboCount      = 0;
	CurrentWidth    = InitialSize;
	CurrentDepth    = InitialSize;
	bNextMoveInX    = true;
	CurrentMoveSpeed = SpeedBase;
	StackCenterX    = 0.f;
	StackCenterY    = 0.f;
	CurrentTopZ     = 0.f;
	bPerfectFlashActive = false;
	PerfectFlashTimer   = 0.f;
	GameState       = EStackGameState::Playing;

	SetupCamera();
	SpawnBaseBlock();
	SpawnNextBlock();
}

void AStackGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bPerfectFlashActive)
	{
		PerfectFlashTimer -= DeltaTime;
		if (PerfectFlashTimer <= 0.f)
		{
			bPerfectFlashActive = false;
			PerfectFlashTimer   = 0.f;
		}
	}
}

float AStackGameMode::GetPerfectFlashAlpha() const
{
	return bPerfectFlashActive ? (PerfectFlashTimer / FlashDuration) : 0.f;
}

// ---------------------------------------------------------------------------
// Camera
// ---------------------------------------------------------------------------

void AStackGameMode::SetupCamera()
{
	FActorSpawnParameters Sp;
	Sp.Owner = this;

	// Position the camera to give a nice diagonal isometric view of the base
	GameCamera = GetWorld()->SpawnActor<ACameraActor>(
		FVector(900.f, -900.f, 1000.f), FRotator(-30.f, 45.f, 0.f), Sp);

	if (GameCamera)
	{
		GameCamera->GetCameraComponent()->FieldOfView = 65.f;
	}

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PC->SetViewTargetWithBlend(GameCamera, 0.f);
	}
}

void AStackGameMode::UpdateCamera()
{
	if (!GameCamera) return;

	// Raise camera with the stack; always look at the top of the stack
	const float TargetZ = CurrentTopZ;
	const float Dist    = 1400.f;
	const float Angle   = 32.f; // degrees of elevation

	FVector CamPos(
		StackCenterX + Dist * FMath::Cos(FMath::DegreesToRadians(Angle)),
		StackCenterY - Dist * FMath::Cos(FMath::DegreesToRadians(Angle)),
		TargetZ + Dist * FMath::Sin(FMath::DegreesToRadians(Angle))
	);

	FVector LookTarget(StackCenterX, StackCenterY, TargetZ - BlockH * 0.5f);
	FRotator LookRot = UKismetMathLibrary::FindLookAtRotation(CamPos, LookTarget);

	GameCamera->SetActorLocation(CamPos);
	GameCamera->SetActorRotation(LookRot);
}

// ---------------------------------------------------------------------------
// Block spawning
// ---------------------------------------------------------------------------

FLinearColor AStackGameMode::BlockColor(int32 Idx) const
{
	if (Idx == 0)
	{
		// Foundation block: dark charcoal
		return FLinearColor(0.18f, 0.18f, 0.22f);
	}
	// Cycle hue every ~30° (256 steps = 360°, so 21 steps ≈ 30°)
	uint8 H = (uint8)((Idx * 21) % 256);
	return FLinearColor::MakeFromHSV8(H, 210, 230);
}

void AStackGameMode::SpawnBaseBlock()
{
	const FVector Pos(0.f, 0.f, BlockH * 0.5f);
	AStackBlock* Base = GetWorld()->SpawnActor<AStackBlock>(
		AStackBlock::StaticClass(), Pos, FRotator::ZeroRotator);

	if (Base)
	{
		Base->InitBlock(InitialSize, InitialSize, Pos, BlockColor(0));
		StackedBlocks.Add(Base);
		CurrentTopZ = BlockH;
		StackCount  = 1;
	}
}

void AStackGameMode::SpawnNextBlock()
{
	if (GameState != EStackGameState::Playing) return;

	const float SpawnZ = CurrentTopZ + BlockH * 0.5f;
	const FVector Origin(StackCenterX, StackCenterY, SpawnZ);

	AStackBlock* New = GetWorld()->SpawnActor<AStackBlock>(
		AStackBlock::StaticClass(), Origin, FRotator::ZeroRotator);

	if (!New) return;

	New->InitBlock(CurrentWidth, CurrentDepth, Origin, BlockColor(StackCount));

	// Movement amplitude: wide enough that the player can miss completely
	const float Range = FMath::Max(InitialSize * 1.4f, 420.f);
	New->StartMoving(bNextMoveInX, CurrentMoveSpeed, Range, Origin);

	ActiveBlock = New;
}

void AStackGameMode::SpawnFallingCutPiece(float CX, float CY, float W, float D,
                                           float Z, FLinearColor Color, FVector Impulse)
{
	const FVector Pos(CX, CY, Z);
	AStackBlock* Cut = GetWorld()->SpawnActor<AStackBlock>(
		AStackBlock::StaticClass(), Pos, FRotator::ZeroRotator);

	if (Cut)
	{
		Cut->InitBlock(W, D, Pos, Color);
		Cut->StartFalling(Impulse);
	}
}

// ---------------------------------------------------------------------------
// Core game logic: PlaceActiveBlock
// ---------------------------------------------------------------------------

void AStackGameMode::PlaceActiveBlock()
{
	if (GameState != EStackGameState::Playing || !ActiveBlock) return;

	ActiveBlock->StopMoving();

	const AStackBlock* Top   = StackedBlocks.Last();
	const FVector      APos  = ActiveBlock->GetActorLocation();
	const FVector      TPos  = Top->GetActorLocation();
	const FLinearColor Color = BlockColor(StackCount);

	float NewW = CurrentWidth;
	float NewD = CurrentDepth;
	float NewCX = TPos.X;
	float NewCY = TPos.Y;
	bool  bPerfect = false;

	static constexpr float PerfectTolerance = 4.f;

	if (bNextMoveInX)
	{
		// Block slid along X — calculate overlap on the X axis
		const float ALeft  = APos.X - ActiveBlock->BlockWidth  * 0.5f;
		const float ARight = APos.X + ActiveBlock->BlockWidth  * 0.5f;
		const float TLeft  = TPos.X - Top->BlockWidth * 0.5f;
		const float TRight = TPos.X + Top->BlockWidth * 0.5f;

		const float OLeft  = FMath::Max(ALeft,  TLeft);
		const float ORight = FMath::Min(ARight, TRight);
		const float Overlap = ORight - OLeft;

		if (Overlap <= 1.f)
		{
			// Complete miss — game over
			ActiveBlock->StartFalling(FVector(0.f, 0.f, 0.f));
			ActiveBlock = nullptr;
			GameState   = EStackGameState::GameOver;
			BestScore   = FMath::Max(BestScore, Score);
			return;
		}

		bPerfect = FMath::Abs(APos.X - TPos.X) < PerfectTolerance;

		if (bPerfect)
		{
			// Perfect alignment: keep same center and size as the block below
			NewCX = TPos.X;
			NewW  = Top->BlockWidth;
		}
		else
		{
			// Cut the hanging portion off
			const float CutW = ActiveBlock->BlockWidth - Overlap;
			const bool  bHangRight = (APos.X > TPos.X);
			const float CutCX = bHangRight ? (ORight + CutW * 0.5f) : (OLeft - CutW * 0.5f);

			SpawnFallingCutPiece(
				CutCX, APos.Y, CutW, ActiveBlock->BlockDepth, APos.Z, Color,
				FVector(bHangRight ? 140.f : -140.f, 0.f, 50.f));

			NewCX = (OLeft + ORight) * 0.5f;
			NewW  = Overlap;
		}

		NewCY = TPos.Y;
		NewD  = Top->BlockDepth; // Y axis not cut this turn
	}
	else
	{
		// Block slid along Y — calculate overlap on the Y axis
		const float ABack  = APos.Y - ActiveBlock->BlockDepth * 0.5f;
		const float AFront = APos.Y + ActiveBlock->BlockDepth * 0.5f;
		const float TBack  = TPos.Y - Top->BlockDepth * 0.5f;
		const float TFront = TPos.Y + Top->BlockDepth * 0.5f;

		const float OBack  = FMath::Max(ABack,  TBack);
		const float OFront = FMath::Min(AFront, TFront);
		const float Overlap = OFront - OBack;

		if (Overlap <= 1.f)
		{
			ActiveBlock->StartFalling(FVector(0.f, 0.f, 0.f));
			ActiveBlock = nullptr;
			GameState   = EStackGameState::GameOver;
			BestScore   = FMath::Max(BestScore, Score);
			return;
		}

		bPerfect = FMath::Abs(APos.Y - TPos.Y) < PerfectTolerance;

		if (bPerfect)
		{
			NewCY = TPos.Y;
			NewD  = Top->BlockDepth;
		}
		else
		{
			const float CutD = ActiveBlock->BlockDepth - Overlap;
			const bool  bHangFront = (APos.Y > TPos.Y);
			const float CutCY = bHangFront ? (OFront + CutD * 0.5f) : (OBack - CutD * 0.5f);

			SpawnFallingCutPiece(
				APos.X, CutCY, ActiveBlock->BlockWidth, CutD, APos.Z, Color,
				FVector(0.f, bHangFront ? 140.f : -140.f, 50.f));

			NewCY = (OBack + OFront) * 0.5f;
			NewD  = Overlap;
		}

		NewCX = TPos.X;
		NewW  = Top->BlockWidth;
	}

	// Clamp block to a playable minimum size
	NewW = FMath::Max(NewW, MinBlockSize);
	NewD = FMath::Max(NewD, MinBlockSize);

	// Place the kept portion at the new stack top
	const float PlacedZ = CurrentTopZ + BlockH * 0.5f;
	ActiveBlock->InitBlock(NewW, NewD, FVector(NewCX, NewCY, PlacedZ), Color);

	StackedBlocks.Add(ActiveBlock);
	ActiveBlock = nullptr;

	// Persist dimensions for next block
	CurrentWidth  = NewW;
	CurrentDepth  = NewD;
	StackCenterX  = NewCX;
	StackCenterY  = NewCY;
	CurrentTopZ  += BlockH;
	StackCount++;

	// Scoring
	if (bPerfect)
	{
		ComboCount++;
		Score += 10 + ComboCount * 5; // bonus grows with streak
		bPerfectFlashActive = true;
		PerfectFlashTimer   = FlashDuration;
	}
	else
	{
		ComboCount = 0;
		Score += 10;
	}

	// Difficulty ramp
	CurrentMoveSpeed = FMath::Min(SpeedBase + StackCount * SpeedPerBlock, SpeedMax);

	// Alternate movement axis
	bNextMoveInX = !bNextMoveInX;

	SpawnNextBlock();
	UpdateCamera();
}

// ---------------------------------------------------------------------------
// Restart
// ---------------------------------------------------------------------------

void AStackGameMode::RestartGame()
{
	ClearAllBlocks();

	Score           = 0;
	StackCount      = 0;
	ComboCount      = 0;
	CurrentWidth    = InitialSize;
	CurrentDepth    = InitialSize;
	bNextMoveInX    = true;
	CurrentMoveSpeed = SpeedBase;
	StackCenterX    = 0.f;
	StackCenterY    = 0.f;
	CurrentTopZ     = 0.f;
	bPerfectFlashActive = false;
	PerfectFlashTimer   = 0.f;
	GameState       = EStackGameState::Playing;

	SpawnBaseBlock();
	SpawnNextBlock();
	UpdateCamera();
}

void AStackGameMode::ClearAllBlocks()
{
	if (ActiveBlock)
	{
		ActiveBlock->Destroy();
		ActiveBlock = nullptr;
	}

	for (AStackBlock* B : StackedBlocks)
	{
		if (IsValid(B)) B->Destroy();
	}
	StackedBlocks.Empty();

	// Also destroy any lingering cut-off pieces (falling actors)
	for (TActorIterator<AStackBlock> It(GetWorld()); It; ++It)
	{
		It->Destroy();
	}
}
