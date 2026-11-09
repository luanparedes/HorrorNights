// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HorrorNightsPlayerController.generated.h"

class UUserWidget;
class UInputAction;
class UInputMappingContext;
class UCheatSubsystem;

UENUM(BlueprintType)
enum class ECheatActivationResult : uint8
{
	InvalidCode			UMETA(DisplayName = "Invalid Code"),
	AlreadyActive		UMETA(DisplayName = "Already Active"),
	AppliedNow			UMETA(DisplayName = "Applied Now"),
	AppliesOnNextSpawn	UMETA(DisplayName = "Applies On Next Spawn"),
	SentToServer		UMETA(DisplayName = "Sent To Server"),
	CharacterSwapped	UMETA(DisplayName = "Character Swapped")
};

UCLASS()
class HORRORNIGHTS_API AHorrorNightsPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Cheats")
	void ToggleCheatWidget();

	UFUNCTION(BlueprintCallable, Category = "Cheats")
	void OpenCheatWidget();

	UFUNCTION(BlueprintCallable, Category = "Cheats")
	void CloseCheatWidget();

	UFUNCTION(BlueprintPure, Category = "Cheats")
	bool IsCheatWidgetOpen() const;

	UFUNCTION(BlueprintCallable, Category = "Cheats")
	ECheatActivationResult RequestActivateCheatByCode(const FString& Code);

	UFUNCTION(BlueprintPure, Category = "Cheats")
	bool IsSwappingCharacter() const { return bIsSwappingCharacter; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Cheats")
	TSubclassOf<UUserWidget> CheatWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Cheats|Input")
	TObjectPtr<UInputAction> ToggleCheatAction;

	UPROPERTY(EditDefaultsOnly, Category = "Cheats|Input")
	TObjectPtr<UInputMappingContext> CheatMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Cheats|Input")
	int32 CheatMappingPriority = 10;

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	UPROPERTY()
	TObjectPtr<UUserWidget> CheatWidgetInstance;

	bool bCursorWasVisible = false;
	bool bIsSwappingCharacter = false;

	UFUNCTION(Server, Reliable)
	void ServerActivateCheatByCode(const FString& Code);

	bool SwapCharacterInPlace();
	UCheatSubsystem* GetCheatSubsystem() const;
	ECheatActivationResult ActivateAndApply(const FString& Code);
};
