// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SpawnTables.h"
#include "AnimalClassData.generated.h"

UCLASS(BlueprintType)
class HORRORNIGHTS_API UAnimalClassData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	FText AnimalName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status", meta = (ClampMin = "1"))
	int32 HP = 100;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status", meta = (ClampMin = "0"))
	int32 Attack = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status", meta = (ClampMin = "0"))
	int32 Defense = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status|Movement", meta = (ClampMin = "0"))
	float WalkSpeed = 150.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status|Movement", meta = (ClampMin = "0"))
	float RunSpeed = 600.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior", meta = (ClampMin = "0"))
	float DetectionRadius = 2000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior", meta = (ClampMin = "0", ClampMax = "1"))
	float AttackChanceWhenApproached = 0.05f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior", meta = (ClampMin = "0", ClampMax = "1"))
	float FleeChanceWhenApproached = 0.8f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior", meta = (ClampMin = "0", ClampMax = "1"))
	float RetaliateChanceWhenHit = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior", meta = (ClampMin = "0"))
	float FleeDistance = 3000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior", meta = (ClampMin = "0"))
	float MinFleeTime = 6.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior|Attack", meta = (ClampMin = "0"))
	float AttackRange = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior|Attack", meta = (ClampMin = "0.1"))
	float AttackCooldown = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior|Attack", meta = (ClampMin = "0"))
	float MaxAttackTime = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wander", meta = (ClampMin = "0"))
	float WanderMinDistance = 800.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wander", meta = (ClampMin = "0"))
	float WanderMaxDistance = 3000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wander", meta = (ClampMin = "0"))
	float IdleTimeMin = 2.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wander", meta = (ClampMin = "0"))
	float IdleTimeMax = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drops")
	TArray<FItemSpawnEntry> Drops;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drops", meta = (ClampMin = "0"))
	int32 MinDropRolls = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drops", meta = (ClampMin = "0"))
	int32 MaxDropRolls = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drops", meta = (ClampMin = "0"))
	float DropScatterRadius = 120.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drops", meta = (ClampMin = "0"))
	float DropLifetime = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drops", meta = (ClampMin = "0.1"))
	float CorpseLifetime = 10.f;
};
