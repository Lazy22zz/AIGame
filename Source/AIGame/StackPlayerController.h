// Stack Up! - StackPlayerController.h
// Handles player input: SPACE to place a block, R to restart.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "StackPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;

UCLASS()
class AIGAME_API AStackPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AStackPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	UPROPERTY()
	UInputAction* PlaceBlockAction;

	UPROPERTY()
	UInputAction* RestartGameAction;

	UPROPERTY()
	UInputMappingContext* StackMappingContext;

	void OnPlaceBlock();
	void OnRestartGame();

	void RegisterMappingContext();
};
