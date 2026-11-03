// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnemyClassData.generated.h"

UCLASS()
class HORRORNIGHTS_API UEnemyClassData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	TSubclassOf<class ACharacter> CharacterBlueprint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	FText CharacterName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status")
	int32 HP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status")
	int32 Attack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status")
	int32 Defense;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status")
	int32 MagicAttack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status")
	int32 Speed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Special Abilities")
	TArray<FName> Abilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Special Abilities")
	TArray<FName> DropItems;
};
