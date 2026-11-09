// Sunnymoon Softwares - All rights reserved

#include "CheatSubsystem.h"
#include "CheatDefinition.h"
#include "PlayerCharacter.h"
#include "HorrorNightsPlayerController.h"
#include "Engine/AssetManager.h"
#include "PlayerClassData.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogCheats, Log, All);

void UCheatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadAvailableCheats();
}

void UCheatSubsystem::LoadAvailableCheats()
{
	AvailableCheats.Reset();

	UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
	if (!AssetManager)
	{
		UE_LOG(LogCheats, Error, TEXT("Asset Manager not initialized, no cheats loaded."));
		return;
	}

	// Pergunta ao Asset Manager por todos os assets do tipo "Cheat" (configurado no DefaultGame.ini).
	TArray<FSoftObjectPath> CheatPaths;
	AssetManager->GetPrimaryAssetPathList(UCheatDefinition::GetCheatAssetType(), CheatPaths);

	for (const FSoftObjectPath& Path : CheatPaths)
	{
		// Data assets pequenos: carregar sincrono na inicializacao eh ok.
		UCheatDefinition* Cheat = Cast<UCheatDefinition>(Path.TryLoad());
		if (!Cheat)
		{
			UE_LOG(LogCheats, Warning, TEXT("Not possible to load cheat %s"), *Path.ToString());
			continue;
		}

		if (Cheat->Code.TrimStartAndEnd().IsEmpty())
		{
			UE_LOG(LogCheats, Warning, TEXT("Cheat %s has no Code and can't be typed."), *Cheat->GetName());
		}
		else if (const UCheatDefinition* Duplicate = FindCheatByCode(Cheat->Code))
		{
			UE_LOG(LogCheats, Warning, TEXT("Code '%s' is used by %s and %s. Only the first one will work."),
				*Cheat->Code, *Duplicate->GetName(), *Cheat->GetName());
		}

		AvailableCheats.Add(Cheat);
	}

	if (CheatPaths.IsEmpty())
	{
		UE_LOG(LogCheats, Warning, TEXT("No cheats found. Check Asset Manager type 'Cheat' and re-save the cheat Data Assets."));
	}

	UE_LOG(LogCheats, Log, TEXT("%d available cheats."), AvailableCheats.Num());
}

UCheatDefinition* UCheatSubsystem::FindCheatByCode(const FString& InCode) const
{
	const FString Code = InCode.TrimStartAndEnd();
	if (Code.IsEmpty())
		return nullptr;

	for (UCheatDefinition* Cheat : AvailableCheats)
	{
		if (Cheat && Cheat->Code.Equals(Code, ESearchCase::IgnoreCase))
		{
			return Cheat;
		}
	}

	return nullptr;
}

bool UCheatSubsystem::TryActivateCheatByCode(const FString& InCode)
{
	UCheatDefinition* Cheat = FindCheatByCode(InCode);
	if (!Cheat)
	{
		UE_LOG(LogCheats, Log, TEXT("Invalid code: %s"), *InCode);
		return false;
	}

	return ActivateCheat(Cheat);
}

bool UCheatSubsystem::ActivateCheat(UCheatDefinition* Cheat)
{
	if (!Cheat)
		return false;

	if (ActiveCheats.Contains(Cheat))
		return true;

	if (!Cheat->ExclusiveGroup.IsNone())
	{
		ActiveCheats.RemoveAll([Cheat](const UCheatDefinition* Other)
		{
			return Other && Other->ExclusiveGroup == Cheat->ExclusiveGroup;
		});
	}

	ActiveCheats.Add(Cheat);
	UE_LOG(LogCheats, Log, TEXT("Cheat ativado: %s (%s)"), *Cheat->GetName(), *Cheat->Code);

	OnActiveCheatsChanged.Broadcast();
	return true;
}

bool UCheatSubsystem::DeactivateCheat(UCheatDefinition* Cheat)
{
	if (ActiveCheats.Remove(Cheat) == 0)
		return false;

	UE_LOG(LogCheats, Log, TEXT("Cheat desativado: %s"), *GetNameSafe(Cheat));
	OnActiveCheatsChanged.Broadcast();
	return true;
}

void UCheatSubsystem::ClearAllCheats()
{
	if (ActiveCheats.IsEmpty())
		return;

	ActiveCheats.Reset();
	UE_LOG(LogCheats, Log, TEXT("All cheats cleared."));
	OnActiveCheatsChanged.Broadcast();
}

bool UCheatSubsystem::IsCheatActive(const UCheatDefinition* Cheat) const
{
	return Cheat && ActiveCheats.Contains(Cheat);
}

TArray<UCheatDefinition*> UCheatSubsystem::GetActiveCheats() const
{
	return ObjectPtrDecay(ActiveCheats);
}

TArray<UCheatDefinition*> UCheatSubsystem::GetAvailableCheats() const
{
	return ObjectPtrDecay(AvailableCheats);
}

UPlayerClassData* UCheatSubsystem::ResolveCharacterClass(UPlayerClassData* DefaultClass)
{
	UPlayerClassData* Result = DefaultClass;

	for (UCheatDefinition* Cheat : ActiveCheats)
	{
		if (Cheat)
		{
			Result = Cheat->ModifyCharacterClass(Result);
		}
	}

	return Result;
}

void UCheatSubsystem::ApplyCheatsToPlayer(APlayerCharacter* Player, bool bIsCharacterSwap)
{
	if (!Player || !Player->HasAuthority())
		return;

	for (UCheatDefinition* Cheat : ActiveCheats)
	{
		// Na troca de personagem, pula o que ja veio transferido (ex: itens no inventario).
		if (Cheat && bIsCharacterSwap && !Cheat->bReapplyOnCharacterSwap)
		{
			continue;
		}

		if (Cheat)
		{
			Cheat->ApplyToPlayer(Player);
			UE_LOG(LogCheats, Log, TEXT("Cheat %s applied to %s"), *Cheat->GetName(), *Player->GetName());
		}
	}
}

// ---------------------------------------------------------------------------
// Comandos de console (abrir com ~):
//   HN.Cheat <codigo>   -> ativa pelo codigo
//   HN.Cheat.List       -> lista disponiveis e ativos
//   HN.Cheat.Clear      -> desativa todos
// ---------------------------------------------------------------------------
namespace CheatConsole
{
	static UCheatSubsystem* GetSubsystem(UWorld* World)
	{
		UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		return GameInstance ? GameInstance->GetSubsystem<UCheatSubsystem>() : nullptr;
	}

	static FAutoConsoleCommandWithWorldAndArgs ActivateCmd(
		TEXT("HN.Cheat"),
		TEXT("Ativa um cheat pelo codigo. Uso: HN.Cheat <codigo>"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			if (!World || Args.Num() == 0)
				return;

			const FString Code = FString::Join(Args, TEXT(" "));

			// Mesmo caminho do WBP: ativa e, se estiver jogando, aplica na hora.
			if (AHorrorNightsPlayerController* PC = Cast<AHorrorNightsPlayerController>(World->GetFirstPlayerController()))
			{
				const ECheatActivationResult Result = PC->RequestActivateCheatByCode(Code);
				UE_LOG(LogCheats, Display, TEXT("HN.Cheat %s -> %s"), *Code, *UEnum::GetDisplayValueAsText(Result).ToString());
			}
			else if (UCheatSubsystem* Cheats = GetSubsystem(World))
			{
				const bool bOk = Cheats->TryActivateCheatByCode(Code);
				UE_LOG(LogCheats, Display, TEXT("%s"), bOk ? TEXT("Cheat ativado!") : TEXT("Codigo invalido."));
			}
		}));

	static FAutoConsoleCommandWithWorldAndArgs ListCmd(
		TEXT("HN.Cheat.List"),
		TEXT("Lista os cheats disponiveis e quais estao ativos."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			UCheatSubsystem* Cheats = GetSubsystem(World);
			if (!Cheats)
				return;

			for (UCheatDefinition* Cheat : Cheats->GetAvailableCheats())
			{
				UE_LOG(LogCheats, Display, TEXT("[%s] %s - %s"),
					Cheats->IsCheatActive(Cheat) ? TEXT("X") : TEXT(" "),
					*Cheat->Code,
					*Cheat->DisplayName.ToString());
			}
		}));

	static FAutoConsoleCommandWithWorldAndArgs ClearCmd(
		TEXT("HN.Cheat.Clear"),
		TEXT("Desativa todos os cheats."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			if (UCheatSubsystem* Cheats = GetSubsystem(World))
			{
				Cheats->ClearAllCheats();
			}
		}));
}
