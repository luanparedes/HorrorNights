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

	// O atalho de cheats tem o proprio Mapping Context, entao funciona com ou sem personagem
	// (no main menu nao existe personagem, e na partida o personagem pode morrer/trocar).
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

	// Volta ao estado anterior: no menu o cursor continua visivel, na partida some.
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

// ---------------------------------------------------------------------------
// Ativacao
// ---------------------------------------------------------------------------

UCheatSubsystem* AHorrorNightsPlayerController::GetCheatSubsystem() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UCheatSubsystem>() : nullptr;
}

ECheatActivationResult AHorrorNightsPlayerController::RequestActivateCheatByCode(const FString& Code)
{
	const UCheatSubsystem* Cheats = GetCheatSubsystem();
	const UCheatDefinition* Cheat = Cheats ? Cheats->FindCheatByCode(Code) : nullptr;

	// Troca de personagem abre a intro do personagem novo, que assume input e cursor.
	// Fecha a caixa antes; se fechar depois, ela esconderia o cursor da intro.
	if (Cheat && Cheat->bRespawnWhenActivatedInGame && GetPawn() && !Cheats->IsCheatActive(Cheat))
	{
		CloseCheatWidget();
	}

	// Single player / host: este controller ja esta no servidor.
	if (HasAuthority())
	{
		return ActivateAndApply(Code);
	}

	// Cliente: valida localmente so pra dar feedback imediato e pede ao servidor.
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

	// Digitar de novo um cheat ja ativo nao reaplica (nao da +50 macas a cada Enter).
	if (Cheats->IsCheatActive(Cheat))
	{
		return ECheatActivationResult::AlreadyActive;
	}

	// Fica ativo para todos os proximos spawns.
	Cheats->ActivateCheat(Cheat);

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetPawn());

	// Cheats como "jogar com personagem X": respawn na hora, no mesmo lugar.
	if (PlayerCharacter && Cheat->bRespawnWhenActivatedInGame)
	{
		return SwapCharacterInPlace()
			? ECheatActivationResult::CharacterSwapped
			: ECheatActivationResult::AppliesOnNextSpawn;
	}

	// Se ja estamos jogando (existe personagem), aplica agora.
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

	// 1) Guarda o que precisa sobreviver a troca.
	const FTransform SpawnTransform(OldCharacter->GetActorRotation(), OldCharacter->GetActorLocation());
	const FRotator SavedControlRotation = GetControlRotation();

	TMap<UItemData*, int32> SavedItems;
	if (OldCharacter->Inventory)
	{
		SavedItems = OldCharacter->Inventory->InventoryItems;
	}

	// Enquanto for true, o personagem novo sabe que eh troca (nao reaplica itens).
	TGuardValue<bool> SwapGuard(bIsSwappingCharacter, true);

	// 2) Remove o personagem atual primeiro, pra liberar o espaco pro novo.
	UnPossess();
	OldCharacter->Destroy();

	// 3) Respawn no mesmo lugar. O GameMode chama GetDefaultPawnClassForController,
	//    que passa pelos cheats e devolve a classe nova. HP/status vem cheios da classe nova.
	GameMode->RestartPlayerAtTransform(this, SpawnTransform);

	// Se nao coube ali (capsula maior, parede...), nasce num PlayerStart pra nunca ficar sem pawn.
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

	// 4) Devolve o inventario.
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
