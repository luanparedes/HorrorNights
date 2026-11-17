// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnemyPower.h"
#include "SpawnTables.generated.h"

class AEnemyCharacter;
class AAnimalCharacter;
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

USTRUCT(BlueprintType)
struct FAnimalSpawnEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	TSubclassOf<AAnimalCharacter> AnimalClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (ClampMin = "0"))
	float Weight = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (ClampMin = "0.05"))
	float MinScale = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (ClampMin = "0.05"))
	float MaxScale = 1.1f;
};

HORRORNIGHTS_API int32 PickWeightedItemEntry(const TArray<FItemSpawnEntry>& Entries);

UCLASS(BlueprintType)
class HORRORNIGHTS_API UMonsterSpawnTable : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monsters")
	TArray<FMonsterSpawnEntry> Entries;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Population", meta = (ClampMin = "0"))
	int32 BaseCount = 150;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Population", meta = (ClampMin = "0"))
	int32 ExtraCountPerNight = 25;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Population", meta = (ClampMin = "0"))
	int32 MaxCount = 400;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Population", meta = (ClampMin = "0", ClampMax = "1"))
	float CountVariance = 0.15f;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Population", meta = (ClampMin = "0"))
	float MinSpacing = 2000.f;

	int32 GetCountForDay() const;
	int32 PickEntryIndex() const;
};

UCLASS(BlueprintType)
class HORRORNIGHTS_API UAnimalSpawnTable : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animals")
	TArray<FAnimalSpawnEntry> Entries;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Population", meta = (ClampMin = "0"))
	int32 MinCount = 60;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Population", meta = (ClampMin = "0"))
	int32 MaxCount = 90;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Population", meta = (ClampMin = "0"))
	float MinSpacing = 2500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Population")
	bool bRefillAtDawn = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Population", meta = (ClampMin = "0"))
	float VirtualWanderSpeed = 60.f;

	int32 GetTargetCount() const;
	int32 PickEntryIndex() const;
};
