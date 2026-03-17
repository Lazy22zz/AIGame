// Stack Up! - StackPlayerController.cpp

#include "StackPlayerController.h"
#include "StackGameMode.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"

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
	StackMappingContext->MapKey(PlaceBlockAction, EKeys::SpaceBar);
	StackMappingContext->MapKey(PlaceBlockAction, EKeys::LeftMouseButton);
	StackMappingContext->MapKey(PlaceBlockAction, EKeys::Gamepad_FaceButton_Bottom);
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
