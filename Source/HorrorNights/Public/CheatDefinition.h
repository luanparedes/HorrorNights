// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CheatDefinition.generated.h"

class APlayerCharacter;
class UPlayerClassData;

UCLASS(Abstract, BlueprintType, Blueprintable)
class HORRORNIGHTS_API UCheatDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cheat")
	FString Code;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cheat")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cheat", meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cheat")
	FName ExclusiveGroup;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cheat")
	bool bCanApplyDuringGameplay = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cheat")
	bool bRespawnWhenActivatedInGame = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cheat")
	bool bReapplyOnCharacterSwap = true;

	UFUNCTION(BlueprintNativeEvent, Category = "Cheat")
	UPlayerClassData* ModifyCharacterClass(UPlayerClassData* CurrentClass);

	UFUNCTION(BlueprintNativeEvent, Category = "Cheat")
	void ApplyToPlayer(APlayerCharacter* Player);

	static FPrimaryAssetType GetCheatAssetType() { return FPrimaryAssetType(TEXT("Cheat")); }
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
