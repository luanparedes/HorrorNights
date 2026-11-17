// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyPower.h"
#include "WorldSpawnDirector.generated.h"

class UBoxComponent;
class UMonsterSpawnTable;
class UItemSpawnTable;
class UAnimalSpawnTable;
class ATimeManager;
class AEnemyCharacter;
class USpawnPresenceComponent;

USTRUCT(BlueprintType)
struct FSpawnExclusionZone
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TObjectPtr<AActor> Center = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0"))
	float Radius = 5000.f;
};

USTRUCT(BlueprintType)
struct FSpawnStreamingRules
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0"))
	float MaterializeDistance = 15000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0"))
	float DematerializeDistance = 18000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0"))
	float ObservationDistance = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0"))
	float MinDistanceFromPlayers = 3000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0"))
	int32 MaxActive = 0;
};

enum class ESpawnCategory : uint8
{
	Monster,
	Item,
	Animal
};

enum class ESpawnRecordState : uint8
{
	Virtual,
	Physical,
	Gone
};

enum class EGroundResult : uint8
{
	Found,
	NotLoaded,
	Invalid
};

struct FSpawnRecord
{
	FVector2D Position = FVector2D::ZeroVector;
	FVector GroundLocation = FVector::ZeroVector;
	bool bGroundResolved = false;
	ESpawnRecordState State = ESpawnRecordState::Virtual;
	TWeakObjectPtr<AActor> Actor;
	int32 EntryIndex = INDEX_NONE;
	double ActivationTime = 0.0;
	float Scale = 1.f;
	bool bOverridePower = false;
	EEnemyPower Power = EEnemyPower::Normal;
	int32 SavedHP = -1;
	int32 Quantity = 1;
	int32 RelocationAttempts = 0;
	FVector2D WanderDirection = FVector2D::ZeroVector;
};

struct FDepartingActor
{
	TWeakObjectPtr<AActor> Actor;
	double Deadline = -1.0;
	ESpawnCategory Category = ESpawnCategory::Item;
};

struct FSpawnViewer
{
	FVector Eye = FVector::ZeroVector;
	FVector Direction = FVector::ForwardVector;
	FVector PawnLocation = FVector::ZeroVector;
	TWeakObjectPtr<AActor> Pawn;
};

UCLASS()
class HORRORNIGHTS_API AWorldSpawnDirector : public AActor
{
	GENERATED_BODY()

public:
	AWorldSpawnDirector();

	UFUNCTION(BlueprintCallable, Category = "Spawn Director")
	void BeginNight();

	UFUNCTION(BlueprintCallable, Category = "Spawn Director")
	void BeginDay();

	UFUNCTION(BlueprintCallable, Category = "Spawn Director")
	void ResetItems();

	UFUNCTION(BlueprintCallable, Category = "Spawn Director")
	void RefillAnimals();

	UFUNCTION(BlueprintPure, Category = "Spawn Director")
	int32 GetNightNumber() const { return NightNumber; }

	UFUNCTION(BlueprintPure, Category = "Spawn Director")
	bool IsNight() const { return bIsNight; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn Director|Area")
	TObjectPtr<UBoxComponent> SpawnArea;

	UPROPERTY(EditAnywhere, Category = "Spawn Director|Area")
	TArray<FSpawnExclusionZone> ExclusionZones;

	UPROPERTY(EditAnywhere, Category = "Spawn Director|Area", meta = (ClampMin = "0"))
	float GuaranteedLoadedRadius = 6000.f;

	UPROPERTY(EditAnywhere, Category = "Spawn Director|Tables")
	TObjectPtr<UMonsterSpawnTable> MonsterTable;

	UPROPERTY(EditAnywhere, Category = "Spawn Director|Tables")
	TObjectPtr<UItemSpawnTable> ItemTable;

	UPROPERTY(EditAnywhere, Category = "Spawn Director|Tables")
	TObjectPtr<UAnimalSpawnTable> AnimalTable;

	UPROPERTY(EditAnywhere, Category = "Spawn Director|Time")
	TObjectPtr<ATimeManager> TimeManager;

	UPROPERTY(EditAnywhere, Category = "Spawn Director|Time")
	bool bSpawnItemsOnBeginPlay = true;

	UPROPERTY(EditAnywhere, Category = "Spawn Director|Time")
	bool bStartAtNight = false;

	UPROPERTY(EditAnywhere, Category = "Spawn Director|Monsters", meta = (ClampMin = "0"))
	float NightArrivalWindow = 120.f;

	UPROPERTY(EditAnywhere, Category = "Spawn Director|Monsters")
	FSpawnStreamingRules MonsterRules;

	UPROPERTY(EditAnywhere, Category = "Spawn Director|Monsters", meta = (ClampMin = "0"))
	float RetreatTimeout = 25.f;

	UPROPERTY(EditAnywhere, Category = "Spawn Director|Monsters", meta = (ClampMin = "0"))
	float RetreatDistance = 6000.f;

	UPROPERTY(EditAnywhere, Category = "Spawn Director|Items")
	FSpawnStreamingRules ItemRules;

	UPROPERTY(EditAnywhere, Category = "Spawn Director|Animals")
	FSpawnStreamingRules AnimalRules;

	UPROPERTY(EditAnywhere, Category = "Spawn Director|Natural", meta = (ClampMin = "1", ClampMax = "180"))
	float ViewHalfAngleDegrees = 75.f;

	UPROPERTY(EditAnywhere, Category = "Spawn Director|Natural", meta = (ClampMin = "1"))
	int32 MaxMaterializationsPerUpdate = 3;

	UPROPERTY(EditAnywhere, Category = "Spawn Director|Natural", meta = (ClampMin = "1"))
	int32 MaxDematerializationsPerUpdate = 6;

	UPROPERTY(EditAnywhere, Category = "Spawn Director|Natural", meta = (ClampMin = "0.05"))
	float UpdateInterval = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Spawn Director|Debug")
	bool bDrawDebug = false;

private:
	TArray<FSpawnRecord> MonsterRecords;
	TArray<FSpawnRecord> ItemRecords;
	TArray<FSpawnRecord> AnimalRecords;
	int32 AnimalTargetCount = 0;
	TArray<FDepartingActor> DepartingActors;

	FTimerHandle UpdateTimer;
	bool bIsNight = false;
	int32 NightNumber = 0;

	void UpdatePopulation();
	void UpdateRecords(TArray<FSpawnRecord>& Records, const FSpawnStreamingRules& Rules, ESpawnCategory Category, const TArray<FSpawnViewer>& Viewers);
	void UpdateDeparting(const TArray<FSpawnViewer>& Viewers);
	void DrawDebugRecords() const;

	void GenerateMonsterRecords();
	void GenerateItemRecords();
	void AddAnimalRecords(int32 Count);
	void WanderVirtualAnimals();
	const FSpawnStreamingRules& GetRules(ESpawnCategory Category) const;

	void GatherViewers(TArray<FSpawnViewer>& OutViewers) const;
	float DistanceToNearestViewer2D(const FVector2D& Position, const TArray<FSpawnViewer>& Viewers) const;
	bool IsObserved(const FVector& Bottom, float Height, float ObservationDistance, const TArray<FSpawnViewer>& Viewers, const AActor* IgnoredActor) const;
	bool IsActorObserved(const AActor* Actor, float ObservationDistance, const TArray<FSpawnViewer>& Viewers) const;

	void ScatterPoints(int32 Count, float MinSpacing, TArray<FVector2D>& OutPoints) const;
	FVector2D RandomPointInArea() const;
	bool IsInsideArea(const FVector2D& Position) const;
	bool IsInsideExclusionZone(const FVector2D& Position) const;
	void RelocateRecord(FSpawnRecord& Record) const;
	EGroundResult ResolveGround(const FVector2D& Position, bool bRequireNavigation, FVector& OutLocation) const;

	AActor* SpawnMonster(const FSpawnRecord& Record);
	AActor* SpawnItem(const FSpawnRecord& Record);
	AActor* SpawnAnimal(const FSpawnRecord& Record);
	void DematerializeRecord(FSpawnRecord& Record, ESpawnCategory Category);
	void SendAway(AActor* Actor, ESpawnCategory Category, const TArray<FSpawnViewer>& Viewers);
	void StartRetreat(AEnemyCharacter* Monster, const TArray<FSpawnViewer>& Viewers) const;

	void TrackActor(AActor* Actor);
	void ReleaseActor(AActor* Actor);
	USpawnPresenceComponent* EnsurePresence(AActor* Actor) const;

	UFUNCTION()
	void HandleSpawnedActorDestroyed(AActor* DestroyedActor);
};
