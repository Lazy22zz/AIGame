// Stack Up! - StackPlayerController.cpp

#include "StackPlayerController.h"
#include "StackGameMode.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputCoreTypes.h"

AStackPlayerController::AStackPlayerController()
{
}

void AStackPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// --- Build Input Actions in code (no .uasset files needed) ---
	PlaceBlockAction = NewObject<UInputAction>(this, TEXT("IA_PlaceBlock"));
	PlaceBlockAction->ValueType = EInputActionValueType::Boolean;

	RestartGameAction = NewObject<UInputAction>(this, TEXT("IA_RestartGame"));
	RestartGameAction->ValueType = EInputActionValueType::Boolean;

	// --- Build Mapping Context ---
	StackMappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_Stack"));

	// Primary action: screen-tap (Android), LMB (desktop), Space, gamepad A
	// Use FKey(TEXT("Touch1")) instead of EKeys::Touch1 to avoid header-order
	// issues; both resolve to the same underlying FName key.
	StackMappingContext->MapKey(PlaceBlockAction, FKey(TEXT("Touch1")));   // finger tap — Android / iOS
	StackMappingContext->MapKey(PlaceBlockAction, EKeys::SpaceBar);
	StackMappingContext->MapKey(PlaceBlockAction, EKeys::LeftMouseButton);
	StackMappingContext->MapKey(PlaceBlockAction, EKeys::Gamepad_FaceButton_Bottom);

	// Dedicated restart — keyboard / gamepad only (touch uses the smart tap path above)
	StackMappingContext->MapKey(RestartGameAction, EKeys::R);
	StackMappingContext->MapKey(RestartGameAction, EKeys::Gamepad_Special_Right);

	RegisterMappingContext();

	// --- Bind actions ---
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EIC->BindAction(PlaceBlockAction,   ETriggerEvent::Started, this, &AStackPlayerController::OnPlaceBlock);
		EIC->BindAction(RestartGameAction,  ETriggerEvent::Started, this, &AStackPlayerController::OnRestartGame);
	}
}

void AStackPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Ensure the mapping context is registered even if SetupInputComponent ran before
	// the local player subsystem was ready.
	RegisterMappingContext();

	// Lock input focus to the game window
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
}

void AStackPlayerController::RegisterMappingContext()
{
	if (!StackMappingContext) return;

	if (UEnhancedInputLocalPlayerSubsystem* Sub =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (!Sub->HasMappingContext(StackMappingContext))
		{
			Sub->AddMappingContext(StackMappingContext, 0);
		}
	}
}

void AStackPlayerController::OnPlaceBlock()
{
	if (AStackGameMode* GM = GetWorld()->GetAuthGameMode<AStackGameMode>())
	{
		// On mobile there is no separate "Restart" key, so a tap does the right
		// thing in every state: restart when the game is over, place otherwise.
		if (GM->IsGameOver())
			GM->RestartGame();
		else
			GM->PlaceActiveBlock();
	}
}

void AStackPlayerController::OnRestartGame()
{
	if (AStackGameMode* GM = GetWorld()->GetAuthGameMode<AStackGameMode>())
	{
		GM->RestartGame();
	}
}
