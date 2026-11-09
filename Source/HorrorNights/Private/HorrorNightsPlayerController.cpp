// Sunnymoon Softwares - All rights reserved

#include "HorrorNightsPlayerController.h"
#include "CheatSubsystem.h"
#include "CheatDefinition.h"
#include "PlayerCharacter.h"
#include "InventoryComponent.h"
#include "ItemData.h"
#include "GameFramework/GameModeBase.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

DEFINE_LOG_CATEGORY_STATIC(LogCheatController, Log, All);

void AHorrorNightsPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() && CheatMappingContext)
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			InputSubsystem->AddMappingContext(CheatMappingContext, CheatMappingPriority);
		}
	}
}

void AHorrorNightsPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (ToggleCheatAction)
		{
			EnhancedInput->BindAction(ToggleCheatAction, ETriggerEvent::Started, this, &AHorrorNightsPlayerController::ToggleCheatWidget);
		}
	}
}

bool AHorrorNightsPlayerController::IsCheatWidgetOpen() const
{
	return CheatWidgetInstance && CheatWidgetInstance->IsInViewport();
}

void AHorrorNightsPlayerController::ToggleCheatWidget()
{
	if (IsCheatWidgetOpen())
	{
		CloseCheatWidget();
	}
	else
	{
		OpenCheatWidget();
	}
}

void AHorrorNightsPlayerController::OpenCheatWidget()
{
	if (!IsLocalController() || IsCheatWidgetOpen())
		return;

	if (!CheatWidgetClass)
	{
		UE_LOG(LogCheatController, Warning, TEXT("CheatWidgetClass is not set on %s."), *GetClass()->GetName());
		return;
	}

	if (!CheatWidgetInstance)
	{
		CheatWidgetInstance = CreateWidget<UUserWidget>(this, CheatWidgetClass);
	}

	if (!CheatWidgetInstance)
		return;

	bCursorWasVisible = ShouldShowMouseCursor();

	CheatWidgetInstance->AddToViewport(100);

	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(CheatWidgetInstance->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	SetShowMouseCursor(true);
}

void AHorrorNightsPlayerController::CloseCheatWidget()
{
	if (!IsCheatWidgetOpen())
		return;

	CheatWidgetInstance->RemoveFromParent();

	if (bCursorWasVisible)
	{
		SetInputMode(FInputModeGameAndUI());
	}
	else
	{
		SetInputMode(FInputModeGameOnly());
	}

	SetShowMouseCursor(bCursorWasVisible);
}

UCheatSubsystem* AHorrorNightsPlayerController::GetCheatSubsystem() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UCheatSubsystem>() : nullptr;
}

ECheatActivationResult AHorrorNightsPlayerController::RequestActivateCheatByCode(const FString& Code)
{
	const UCheatSubsystem* Cheats = GetCheatSubsystem();
	const UCheatDefinition* Cheat = Cheats ? Cheats->FindCheatByCode(Code) : nullptr;

	if (Cheat && Cheat->bRespawnWhenActivatedInGame && GetPawn() && !Cheats->IsCheatActive(Cheat))
	{
		CloseCheatWidget();
	}

	if (HasAuthority())
	{
		return ActivateAndApply(Code);
	}

	if (!Cheat)
	{
		return ECheatActivationResult::InvalidCode;
	}

	ServerActivateCheatByCode(Code);
	return ECheatActivationResult::SentToServer;
}

void AHorrorNightsPlayerController::ServerActivateCheatByCode_Implementation(const FString& Code)
{
	ActivateAndApply(Code);
}

ECheatActivationResult AHorrorNightsPlayerController::ActivateAndApply(const FString& Code)
{
	UCheatSubsystem* Cheats = GetCheatSubsystem();
	UCheatDefinition* Cheat = Cheats ? Cheats->FindCheatByCode(Code) : nullptr;

	if (!Cheat)
	{
		return ECheatActivationResult::InvalidCode;
	}

	if (Cheats->IsCheatActive(Cheat))
	{
		return ECheatActivationResult::AlreadyActive;
	}

	Cheats->ActivateCheat(Cheat);

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetPawn());

	if (PlayerCharacter && Cheat->bRespawnWhenActivatedInGame)
	{
		return SwapCharacterInPlace()
			? ECheatActivationResult::CharacterSwapped
			: ECheatActivationResult::AppliesOnNextSpawn;
	}

	if (PlayerCharacter && Cheat->bCanApplyDuringGameplay)
	{
		Cheat->ApplyToPlayer(PlayerCharacter);
		UE_LOG(LogCheatController, Log, TEXT("Cheat %s applied now to %s"), *Cheat->GetName(), *PlayerCharacter->GetName());
		return ECheatActivationResult::AppliedNow;
	}

	return ECheatActivationResult::AppliesOnNextSpawn;
}

bool AHorrorNightsPlayerController::SwapCharacterInPlace()
{
	APlayerCharacter* OldCharacter = Cast<APlayerCharacter>(GetPawn());
	UWorld* World = GetWorld();
	AGameModeBase* GameMode = World ? World->GetAuthGameMode() : nullptr;

	if (!HasAuthority() || !OldCharacter || !GameMode)
		return false;

	const FTransform SpawnTransform(OldCharacter->GetActorRotation(), OldCharacter->GetActorLocation());
	const FRotator SavedControlRotation = GetControlRotation();

	TMap<UItemData*, int32> SavedItems;
	if (OldCharacter->Inventory)
	{
		SavedItems = OldCharacter->Inventory->InventoryItems;
	}

	TGuardValue<bool> SwapGuard(bIsSwappingCharacter, true);

	UnPossess();
	OldCharacter->Destroy();

	GameMode->RestartPlayerAtTransform(this, SpawnTransform);

	if (!GetPawn())
	{
		UE_LOG(LogCheatController, Warning, TEXT("Could not respawn in place, using a PlayerStart."));
		GameMode->RestartPlayer(this);
	}

	APlayerCharacter* NewCharacter = Cast<APlayerCharacter>(GetPawn());
	if (!NewCharacter)
	{
		UE_LOG(LogCheatController, Error, TEXT("Character swap failed: no new character was spawned."));
		return false;
	}

	SetControlRotation(SavedControlRotation);

	if (NewCharacter->Inventory)
	{
		for (const TPair<UItemData*, int32>& Pair : SavedItems)
		{
			NewCharacter->Inventory->AddItem(Pair.Key, Pair.Value);
		}
	}

	UE_LOG(LogCheatController, Log, TEXT("Character swapped to %s"), *NewCharacter->GetName());
	return true;
}
