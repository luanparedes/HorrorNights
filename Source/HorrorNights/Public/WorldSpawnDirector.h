// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyPower.h"
#include "WorldSpawnDirector.generated.h"

class UBoxComponent;
class UMonsterSpawnTable;
class UItemSpawnTable;
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

	// Abaixo desta distância de algum jogador o ator físico é criado
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0"))
	float MaterializeDistance = 15000.f;

	// Acima desta distância de todos os jogadores o ator volta a ser só uma ficha
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0"))
	float DematerializeDistance = 18000.f;

	// Até onde um jogador consegue enxergar este tipo de ator
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0"))
	float ObservationDistance = 10000.f;

	// Nunca nasce mais perto que isso de um jogador, mesmo fora da visão
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0"))
	float MinDistanceFromPlayers = 3000.f;

	// Limite de atores físicos ao mesmo tempo (0 = sem limite)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0"))
	int32 MaxActive = 0;
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

// Ficha de um monstro ou item: existe no mapa inteiro, mas só vira ator perto dos jogadores
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
};

struct FDepartingActor
{
	TWeakObjectPtr<AActor> Actor;
	double Deadline = -1.0;
	bool bIsMonster = false;
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

	UFUNCTION(BlueprintPure, Category = "Spawn Director")
	int32 GetNightNumber() const { return NightNumber; }

	UFUNCTION(BlueprintPure, Category = "Spawn Director")
	bool IsNight() const { return bIsNight; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Estique esta caixa para cobrir todo o cenário jogável
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn Director|Area")
	TObjectPtr<UBoxComponent> SpawnArea;

	// Áreas onde nada nasce (ex.: a cabana)
	UPROPERTY(EditAnywhere, Category = "Spawn Director|Area")
	TArray<FSpawnExclusionZone> ExclusionZones;

	// Perto assim de um jogador o chão com certeza está carregado
	UPROPERTY(EditAnywhere, Category = "Spawn Director|Area", meta = (ClampMin = "0"))
	float GuaranteedLoadedRadius = 6000.f;

	UPROPERTY(EditAnywhere, Category = "Spawn Director|Tables")
	TObjectPtr<UMonsterSpawnTable> MonsterTable;

	UPROPERTY(EditAnywhere, Category = "Spawn Director|Tables")
	TObjectPtr<UItemSpawnTable> ItemTable;

	// Vazio = procura o TimeManager do mapa sozinho
	UPROPERTY(EditAnywhere, Category = "Spawn Director|Time")
	TObjectPtr<ATimeManager> TimeManager;

	UPROPERTY(EditAnywhere, Category = "Spawn Director|Time")
	bool bSpawnItemsOnBeginPlay = true;

	UPROPERTY(EditAnywhere, Category = "Spawn Director|Time")
	bool bStartAtNight = false;

	// Os monstros chegam aos poucos ao longo deste tempo depois que a noite começa
	UPROPERTY(EditAnywhere, Category = "Spawn Director|Monsters", meta = (ClampMin = "0"))
	float NightArrivalWindow = 120.f;

	UPROPERTY(EditAnywhere, Category = "Spawn Director|Monsters")
	FSpawnStreamingRules MonsterRules;

	// Ao amanhecer, monstros sendo vistos fogem; depois deste tempo somem em fade
	UPROPERTY(EditAnywhere, Category = "Spawn Director|Monsters", meta = (ClampMin = "0"))
	float RetreatTimeout = 25.f;

	UPROPERTY(EditAnywhere, Category = "Spawn Director|Monsters", meta = (ClampMin = "0"))
	float RetreatDistance = 6000.f;

	UPROPERTY(EditAnywhere, Category = "Spawn Director|Items")
	FSpawnStreamingRules ItemRules;

	// Metade do ângulo de visão considerado ao checar se o jogador está olhando
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
	TArray<FDepartingActor> DepartingActors;

	FTimerHandle UpdateTimer;
	bool bIsNight = false;
	int32 NightNumber = 0;

	void UpdatePopulation();
	void UpdateRecords(TArray<FSpawnRecord>& Records, const FSpawnStreamingRules& Rules, bool bMonsters, const TArray<FSpawnViewer>& Viewers);
	void UpdateDeparting(const TArray<FSpawnViewer>& Viewers);
	void DrawDebugRecords() const;

	void GenerateMonsterRecords();
	void GenerateItemRecords();

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
	void DematerializeRecord(FSpawnRecord& Record, bool bMonsters);
	void SendAway(AActor* Actor, bool bIsMonster, const TArray<FSpawnViewer>& Viewers);
	void StartRetreat(AEnemyCharacter* Monster, const TArray<FSpawnViewer>& Viewers) const;

	void TrackActor(AActor* Actor);
	void ReleaseActor(AActor* Actor);
	USpawnPresenceComponent* EnsurePresence(AActor* Actor) const;

	UFUNCTION()
	void HandleSpawnedActorDestroyed(AActor* DestroyedActor);
};
