// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyCharacter.h"
#include "EnemyIntelligentSpawner.generated.h"

USTRUCT()
struct FVirtualWolfData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FVector CurrentLocation;

	UPROPERTY()
	float CurrentHP;

	UPROPERTY()
	EEnemyPower EnemyPower;

	UPROPERTY()
	float EnemyScale;

	UPROPERTY()
	bool bIsSpawned;

	UPROPERTY()
	AEnemyCharacter* PhysicalWolf;

	// O HP começa em -1.0f para sabermos que ele nunca tomou dano ainda
	FVirtualWolfData() : CurrentLocation(FVector::ZeroVector), CurrentHP(-1.0f), EnemyPower(static_cast<EEnemyPower>(0)), EnemyScale(1.0f), bIsSpawned(false), PhysicalWolf(nullptr) {}
};

UCLASS()
class HORRORNIGHTS_API AEnemyIntelligentSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	FTimerHandle NightTimerHandle;
	bool bIsNightTime = false;

	UPROPERTY()
	TArray<FVirtualWolfData> NightWolves;

	UPROPERTY(EditAnywhere, Category = "SpawnConfig")
	TSubclassOf<class AEnemyCharacter> EnemyClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpawnConfig")
	TArray<AEnemyCharacter*> LiveEnemies;

	UPROPERTY(EditAnywhere)
	TArray<AActor*> SpawnAnchors;

	UPROPERTY(EditAnywhere, Category = "SpawnConfig")
	AActor* CabinTargetPoint;

	UPROPERTY(EditAnywhere, Category = "SpawnConfig")
	float MinimumDistanceAwayFromCabin = 10000.0f;

	AEnemyIntelligentSpawner();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Time Cycle")
	void BeginNight();

	UFUNCTION(BlueprintCallable, Category = "Time Cycle")
	void BeginDay();

	UFUNCTION()
	void ProcessNightSpawns();

protected:
	virtual void BeginPlay() override;

private:	

};
