// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnemyPower.h"
#include "SpawnTables.generated.h"

class AEnemyCharacter;
class AItem;
class UItemData;

USTRUCT(BlueprintType)
struct FEnemyPowerWeight
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	EEnemyPower Power = EEnemyPower::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (ClampMin = "0"))
	float Weight = 1.f;
};

USTRUCT(BlueprintType)
struct FMonsterSpawnEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	TSubclassOf<AEnemyCharacter> MonsterClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (ClampMin = "0"))
	float Weight = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (ClampMin = "1"))
	int32 FirstNight = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (ClampMin = "0.05"))
	float MinScale = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (ClampMin = "0.05"))
	float MaxScale = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	TArray<FEnemyPowerWeight> PowerWeights;

	bool PickPower(EEnemyPower& OutPower) const;
};

USTRUCT(BlueprintType)
struct FItemSpawnEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	TSubclassOf<AItem> ItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	TObjectPtr<UItemData> ItemDataOverride = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (ClampMin = "0"))
	float Weight = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (ClampMin = "1"))
	int32 MinQuantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (ClampMin = "1"))
	int32 MaxQuantity = 1;
};

UCLASS(BlueprintType)
class HORRORNIGHTS_API UMonsterSpawnTable : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monsters")
	TArray<FMonsterSpawnEntry> Entries;

	// Quantidade de monstros espalhados pelo mapa na primeira noite
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Population", meta = (ClampMin = "0"))
	int32 BaseCount = 150;

	// Quantos monstros a mais a cada noite que passa
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Population", meta = (ClampMin = "0"))
	int32 ExtraCountPerNight = 25;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Population", meta = (ClampMin = "0"))
	int32 MaxCount = 400;

	// Variação aleatória da quantidade (0.15 = +-15%)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Population", meta = (ClampMin = "0", ClampMax = "1"))
	float CountVariance = 0.15f;

	// Distância mínima entre dois monstros na hora de espalhar pelo mapa
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Population", meta = (ClampMin = "0"))
	float MinSpacing = 1500.f;

	int32 GetCountForNight(int32 NightNumber) const;
	int32 PickEntryIndex(int32 NightNumber) const;
};

UCLASS(BlueprintType)
class HORRORNIGHTS_API UItemSpawnTable : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Items")
	TArray<FItemSpawnEntry> Entries;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Population", meta = (ClampMin = "0"))
	int32 MinCount = 60;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Population", meta = (ClampMin = "0"))
	int32 MaxCount = 100;

	// Distância mínima entre dois itens na hora de espalhar pelo mapa
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Population", meta = (ClampMin = "0"))
	float MinSpacing = 2000.f;

	int32 GetCountForDay() const;
	int32 PickEntryIndex() const;
};
