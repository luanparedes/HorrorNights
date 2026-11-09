// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CheatSubsystem.generated.h"

class UCheatDefinition;
class UPlayerClassData;
class APlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActiveCheatsChanged);

UCLASS(BlueprintType)
class HORRORNIGHTS_API UCheatSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Cheats")
	FOnActiveCheatsChanged OnActiveCheatsChanged;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Cheats")
	bool TryActivateCheatByCode(const FString& InCode);

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

	UPlayerClassData* ResolveCharacterClass(UPlayerClassData* DefaultClass);
	void ApplyCheatsToPlayer(APlayerCharacter* Player, bool bIsCharacterSwap = false);

private:
	UPROPERTY()
	TArray<TObjectPtr<UCheatDefinition>> AvailableCheats;

	UPROPERTY()
	TArray<TObjectPtr<UCheatDefinition>> ActiveCheats;

	void LoadAvailableCheats();
};
