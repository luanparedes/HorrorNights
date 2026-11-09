// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CheatSubsystem.generated.h"

class UCheatDefinition;
class UPlayerClassData;
class APlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActiveCheatsChanged);

/**
 * Guarda quais cheats estao ativos e aplica eles quando o jogador nasce.
 * A lista de cheats existentes vem do Asset Manager (tipo "Cheat"), sem lista manual.
 *
 * Por ser um GameInstanceSubsystem, sobrevive a troca de mapa: o jogador ativa
 * o cheat no menu principal e ele continua valendo quando a partida carrega.
 *
 * Fluxo:
 *  Menu (UI ou console) -> TryActivateCheatByCode("CODIGO")
 *  GameMode::GetDefaultPawnClassForController -> ResolveCharacterClass()
 *  PlayerCharacter::TryFinishSetup             -> ApplyCheatsToPlayer()
 */
UCLASS(BlueprintType)
class HORRORNIGHTS_API UCheatSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Procura um cheat pelo codigo digitado e ativa. Retorna false se o codigo nao existe. */
	UFUNCTION(BlueprintCallable, Category = "Cheats")
	bool TryActivateCheatByCode(const FString& InCode);

	/** Ativa um cheat. Desativa outros cheats do mesmo ExclusiveGroup. */
	UFUNCTION(BlueprintCallable, Category = "Cheats")
	bool ActivateCheat(UCheatDefinition* Cheat);

	UFUNCTION(BlueprintCallable, Category = "Cheats")
	bool DeactivateCheat(UCheatDefinition* Cheat);

	UFUNCTION(BlueprintCallable, Category = "Cheats")
	void ClearAllCheats();

	UFUNCTION(BlueprintPure, Category = "Cheats")
	bool IsCheatActive(const UCheatDefinition* Cheat) const;

	UFUNCTION(BlueprintPure, Category = "Cheats")
	TArray<UCheatDefinition*> GetActiveCheats() const;

	UFUNCTION(BlueprintPure, Category = "Cheats")
	TArray<UCheatDefinition*> GetAvailableCheats() const;

	UFUNCTION(BlueprintPure, Category = "Cheats")
	UCheatDefinition* FindCheatByCode(const FString& InCode) const;

	/** Passa a classe sorteada/escolhida por todos os cheats ativos, em ordem de ativacao. */
	UPlayerClassData* ResolveCharacterClass(UPlayerClassData* DefaultClass);

	/** Aplica todos os cheats ativos no jogador. Chamar no servidor. */
	void ApplyCheatsToPlayer(APlayerCharacter* Player, bool bIsCharacterSwap = false);

	/** Disparado sempre que a lista de cheats ativos muda (bom pra atualizar a UI). */
	UPROPERTY(BlueprintAssignable, Category = "Cheats")
	FOnActiveCheatsChanged OnActiveCheatsChanged;

private:
	UPROPERTY()
	TArray<TObjectPtr<UCheatDefinition>> AvailableCheats;

	UPROPERTY()
	TArray<TObjectPtr<UCheatDefinition>> ActiveCheats;

	void LoadAvailableCheats();
};
