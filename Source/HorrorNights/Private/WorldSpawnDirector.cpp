// Sunnymoon Softwares - All rights reserved


#include "WorldSpawnDirector.h"
#include "SpawnTables.h"
#include "SpawnPresenceComponent.h"
#include "TimeManager.h"
#include "EnemyCharacter.h"
#include "Item.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "NavigationSystem.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"

DEFINE_LOG_CATEGORY_STATIC(LogSpawnDirector, Log, All);

AWorldSpawnDirector::AWorldSpawnDirector()
{
	PrimaryActorTick.bCanEverTick = false;

	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
	RootComponent = SpawnArea;
	SpawnArea->SetBoxExtent(FVector(50000.f, 50000.f, 20000.f));
	SpawnArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpawnArea->SetGenerateOverlapEvents(false);
	SpawnArea->SetCanEverAffectNavigation(false);
	SpawnArea->ShapeColor = FColor(255, 140, 0);

	MonsterRules.MaterializeDistance = 15000.f;
	MonsterRules.DematerializeDistance = 18000.f;
	MonsterRules.ObservationDistance = 10000.f;
	MonsterRules.MinDistanceFromPlayers = 3000.f;
	MonsterRules.MaxActive = 40;

	ItemRules.MaterializeDistance = 10000.f;
	ItemRules.DematerializeDistance = 13000.f;
	ItemRules.ObservationDistance = 6000.f;
	ItemRules.MinDistanceFromPlayers = 1500.f;
	ItemRules.MaxActive = 0;
}

void AWorldSpawnDirector::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	if (!TimeManager)
	{
		for (TActorIterator<ATimeManager> It(GetWorld()); It; ++It)
		{
			TimeManager = *It;
			break;
		}
	}

	if (TimeManager)
	{
		TimeManager->OnNightStarted.AddDynamic(this, &AWorldSpawnDirector::BeginNight);
		TimeManager->OnDayStarted.AddDynamic(this, &AWorldSpawnDirector::BeginDay);
	}
	else
	{
		UE_LOG(LogSpawnDirector, Warning, TEXT("Nenhum TimeManager encontrado: chame BeginNight/BeginDay manualmente."));
	}

	GetWorldTimerManager().SetTimer(UpdateTimer, this, &AWorldSpawnDirector::UpdatePopulation, UpdateInterval, true);

	if (bSpawnItemsOnBeginPlay)
	{
		GenerateItemRecords();
	}

	if (bStartAtNight)
	{
		BeginNight();
	}
}

void AWorldSpawnDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(UpdateTimer);

	if (TimeManager)
	{
		TimeManager->OnNightStarted.RemoveDynamic(this, &AWorldSpawnDirector::BeginNight);
		TimeManager->OnDayStarted.RemoveDynamic(this, &AWorldSpawnDirector::BeginDay);
	}

	Super::EndPlay(EndPlayReason);
}

// ---------------------------------------------------------------------------
// Ciclo dia / noite
// ---------------------------------------------------------------------------

void AWorldSpawnDirector::BeginNight()
{
	if (!HasAuthority() || bIsNight)
	{
		return;
	}

	bIsNight = true;
	++NightNumber;
	GenerateMonsterRecords();

	UE_LOG(LogSpawnDirector, Log, TEXT("Noite %d: %d monstros espalhados pelo mapa."), NightNumber, MonsterRecords.Num());
}

void AWorldSpawnDirector::BeginDay()
{
	if (!HasAuthority())
	{
		return;
	}

	bIsNight = false;

	TArray<FSpawnViewer> Viewers;
	GatherViewers(Viewers);

	for (FSpawnRecord& Record : MonsterRecords)
	{
		if (Record.State == ESpawnRecordState::Physical)
		{
			if (AActor* Monster = Record.Actor.Get())
			{
				SendAway(Monster, true, Viewers);
			}
		}
	}
	MonsterRecords.Reset();

	ResetItems();
}

void AWorldSpawnDirector::ResetItems()
{
	if (!HasAuthority())
	{
		return;
	}

	TArray<FSpawnViewer> Viewers;
	GatherViewers(Viewers);

	for (FSpawnRecord& Record : ItemRecords)
	{
		if (Record.State == ESpawnRecordState::Physical)
		{
			if (AActor* Item = Record.Actor.Get())
			{
				SendAway(Item, false, Viewers);
			}
		}
	}

	GenerateItemRecords();
}

// ---------------------------------------------------------------------------
// Geração das fichas
// ---------------------------------------------------------------------------

void AWorldSpawnDirector::GenerateMonsterRecords()
{
	MonsterRecords.Reset();

	if (!MonsterTable)
	{
		UE_LOG(LogSpawnDirector, Warning, TEXT("MonsterTable não configurada."));
		return;
	}

	TArray<FVector2D> Points;
	ScatterPoints(MonsterTable->GetCountForNight(NightNumber), MonsterTable->MinSpacing, Points);

	const double Now = GetWorld()->GetTimeSeconds();

	for (const FVector2D& Point : Points)
	{
		const int32 EntryIndex = MonsterTable->PickEntryIndex(NightNumber);
		if (EntryIndex == INDEX_NONE)
		{
			break;
		}

		const FMonsterSpawnEntry& Entry = MonsterTable->Entries[EntryIndex];

		FSpawnRecord& Record = MonsterRecords.AddDefaulted_GetRef();
		Record.Position = Point;
		Record.EntryIndex = EntryIndex;
		Record.ActivationTime = Now + FMath::FRandRange(0.f, NightArrivalWindow);
		Record.Scale = FMath::FRandRange(FMath::Min(Entry.MinScale, Entry.MaxScale), FMath::Max(Entry.MinScale, Entry.MaxScale));
		Record.bOverridePower = Entry.PickPower(Record.Power);
	}
}

void AWorldSpawnDirector::GenerateItemRecords()
{
	ItemRecords.Reset();

	if (!ItemTable)
	{
		UE_LOG(LogSpawnDirector, Warning, TEXT("ItemTable não configurada."));
		return;
	}

	TArray<FVector2D> Points;
	ScatterPoints(ItemTable->GetCountForDay(), ItemTable->MinSpacing, Points);

	const double Now = GetWorld()->GetTimeSeconds();

	for (const FVector2D& Point : Points)
	{
		const int32 EntryIndex = ItemTable->PickEntryIndex();
		if (EntryIndex == INDEX_NONE)
		{
			break;
		}

		const FItemSpawnEntry& Entry = ItemTable->Entries[EntryIndex];

		FSpawnRecord& Record = ItemRecords.AddDefaulted_GetRef();
		Record.Position = Point;
		Record.EntryIndex = EntryIndex;
		Record.ActivationTime = Now;
		Record.Quantity = FMath::RandRange(FMath::Min(Entry.MinQuantity, Entry.MaxQuantity), FMath::Max(Entry.MinQuantity, Entry.MaxQuantity));
	}

	UE_LOG(LogSpawnDirector, Log, TEXT("Novo dia: %d itens espalhados pelo mapa."), ItemRecords.Num());
}

// ---------------------------------------------------------------------------
// Distribuição pelo cenário
// ---------------------------------------------------------------------------

FVector2D AWorldSpawnDirector::RandomPointInArea() const
{
	const FVector Extent = SpawnArea->GetScaledBoxExtent();
	const FVector Local(FMath::FRandRange(-Extent.X, Extent.X), FMath::FRandRange(-Extent.Y, Extent.Y), 0.f);

	return FVector2D(SpawnArea->GetComponentTransform().TransformPositionNoScale(Local));
}

bool AWorldSpawnDirector::IsInsideArea(const FVector2D& Position) const
{
	const FTransform& AreaTransform = SpawnArea->GetComponentTransform();
	const FVector Local = AreaTransform.InverseTransformPositionNoScale(FVector(Position, AreaTransform.GetLocation().Z));
	const FVector Extent = SpawnArea->GetScaledBoxExtent();

	return FMath::Abs(Local.X) <= Extent.X && FMath::Abs(Local.Y) <= Extent.Y;
}

bool AWorldSpawnDirector::IsInsideExclusionZone(const FVector2D& Position) const
{
	for (const FSpawnExclusionZone& Zone : ExclusionZones)
	{
		if (Zone.Center && FVector2D::DistSquared(Position, FVector2D(Zone.Center->GetActorLocation())) < FMath::Square(Zone.Radius))
		{
			return true;
		}
	}
	return false;
}

// Espalha pontos de forma uniforme pelo cenário inteiro, mantendo uma distância
// mínima entre eles (evita os grupos que nasciam juntos nas âncoras).
void AWorldSpawnDirector::ScatterPoints(int32 Count, float MinSpacing, TArray<FVector2D>& OutPoints) const
{
	OutPoints.Reset();

	if (Count <= 0)
	{
		return;
	}

	float Spacing = FMath::Max(MinSpacing, 1.f);
	TMap<FIntPoint, TArray<int32>> Grid;

	auto CellOf = [&Spacing](const FVector2D& Point)
	{
		return FIntPoint(FMath::FloorToInt(Point.X / Spacing), FMath::FloorToInt(Point.Y / Spacing));
	};

	// Se a área for pequena demais para o espaçamento pedido, ele é reduzido aos poucos
	for (int32 Pass = 0; Pass < 4 && OutPoints.Num() < Count; ++Pass)
	{
		Grid.Reset();
		for (int32 Index = 0; Index < OutPoints.Num(); ++Index)
		{
			Grid.FindOrAdd(CellOf(OutPoints[Index])).Add(Index);
		}

		const float SpacingSquared = Spacing * Spacing;
		int32 Attempts = (Count - OutPoints.Num()) * 30;

		while (Attempts-- > 0 && OutPoints.Num() < Count)
		{
			const FVector2D Candidate = RandomPointInArea();

			if (IsInsideExclusionZone(Candidate))
			{
				continue;
			}

			const FIntPoint Cell = CellOf(Candidate);
			bool bTooClose = false;

			for (int32 DX = -1; DX <= 1 && !bTooClose; ++DX)
			{
				for (int32 DY = -1; DY <= 1 && !bTooClose; ++DY)
				{
					if (const TArray<int32>* Bucket = Grid.Find(Cell + FIntPoint(DX, DY)))
					{
						for (const int32 Other : *Bucket)
						{
							if (FVector2D::DistSquared(OutPoints[Other], Candidate) < SpacingSquared)
							{
								bTooClose = true;
								break;
							}
						}
					}
				}
			}

			if (!bTooClose)
			{
				Grid.FindOrAdd(Cell).Add(OutPoints.Add(Candidate));
			}
		}

		Spacing *= 0.7f;
	}

	if (OutPoints.Num() < Count)
	{
		UE_LOG(LogSpawnDirector, Warning, TEXT("Só couberam %d de %d pontos na área de spawn."), OutPoints.Num(), Count);
	}
}

void AWorldSpawnDirector::RelocateRecord(FSpawnRecord& Record) const
{
	if (++Record.RelocationAttempts > 5)
	{
		Record.State = ESpawnRecordState::Gone;
		return;
	}

	for (int32 Try = 0; Try < 10; ++Try)
	{
		const FVector2D Candidate = Record.Position + FVector2D(FMath::VRand()).GetSafeNormal() * FMath::FRandRange(1000.f, 4000.f);

		if (IsInsideArea(Candidate) && !IsInsideExclusionZone(Candidate))
		{
			Record.Position = Candidate;
			Record.bGroundResolved = false;
			return;
		}
	}

	Record.Position = RandomPointInArea();
	Record.bGroundResolved = false;
}

EGroundResult AWorldSpawnDirector::ResolveGround(const FVector2D& Position, bool bRequireNavigation, FVector& OutLocation) const
{
	const FVector Center = SpawnArea->GetComponentLocation();
	const float HalfHeight = SpawnArea->GetScaledBoxExtent().Z;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(SpawnDirectorGround), false, this);
	FHitResult Hit;

	const FVector Start(Position, Center.Z + HalfHeight);
	const FVector End(Position, Center.Z - HalfHeight);

	if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
	{
		return EGroundResult::NotLoaded;
	}

	// Projeta no NavMesh: corrige pontos que caíram em cima de árvores ou pedras
	// e garante que o monstro consegue andar dali
	if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
	{
		FNavLocation NavLocation;
		if (NavSys->ProjectPointToNavigation(Hit.ImpactPoint, NavLocation, FVector(800.f, 800.f, 3000.f)))
		{
			OutLocation = NavLocation.Location;
			return EGroundResult::Found;
		}
	}

	if (!bRequireNavigation && Hit.ImpactNormal.Z > 0.7f)
	{
		OutLocation = Hit.ImpactPoint;
		return EGroundResult::Found;
	}

	return EGroundResult::Invalid;
}

// ---------------------------------------------------------------------------
// Visão dos jogadores
// ---------------------------------------------------------------------------

void AWorldSpawnDirector::GatherViewers(TArray<FSpawnViewer>& OutViewers) const
{
	OutViewers.Reset();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();
		APawn* Pawn = PlayerController ? PlayerController->GetPawn() : nullptr;

		if (!Pawn)
		{
			continue;
		}

		FVector Eye;
		FRotator ViewRotation;
		PlayerController->GetPlayerViewPoint(Eye, ViewRotation);

		FSpawnViewer& Viewer = OutViewers.AddDefaulted_GetRef();
		Viewer.Eye = Eye;
		Viewer.Direction = ViewRotation.Vector();
		Viewer.PawnLocation = Pawn->GetActorLocation();
		Viewer.Pawn = Pawn;
	}
}

float AWorldSpawnDirector::DistanceToNearestViewer2D(const FVector2D& Position, const TArray<FSpawnViewer>& Viewers) const
{
	float Nearest = TNumericLimits<float>::Max();

	for (const FSpawnViewer& Viewer : Viewers)
	{
		Nearest = FMath::Min(Nearest, FVector2D::Distance(Position, FVector2D(Viewer.PawnLocation)));
	}
	return Nearest;
}

bool AWorldSpawnDirector::IsObserved(const FVector& Bottom, float Height, float ObservationDistance, const TArray<FSpawnViewer>& Viewers, const AActor* IgnoredActor) const
{
	const float CosHalfAngle = FMath::Cos(FMath::DegreesToRadians(ViewHalfAngleDegrees));
	const FVector Points[2] = { Bottom + FVector(0.f, 0.f, Height * 0.5f), Bottom + FVector(0.f, 0.f, Height) };

	for (const FSpawnViewer& Viewer : Viewers)
	{
		FCollisionQueryParams Params(SCENE_QUERY_STAT(SpawnDirectorVisibility), false, this);
		Params.AddIgnoredActor(Viewer.Pawn.Get());
		if (IgnoredActor)
		{
			Params.AddIgnoredActor(IgnoredActor);
		}

		for (const FVector& Point : Points)
		{
			const FVector ToPoint = Point - Viewer.Eye;
			const float Distance = ToPoint.Size();

			if (Distance > ObservationDistance)
			{
				continue;
			}

			if (Distance < 1.f)
			{
				return true;
			}

			if (FVector::DotProduct(ToPoint / Distance, Viewer.Direction) < CosHalfAngle)
			{
				continue;
			}

			if (!GetWorld()->LineTraceTestByChannel(Viewer.Eye, Point, ECC_Visibility, Params))
			{
				return true;
			}
		}
	}
	return false;
}

bool AWorldSpawnDirector::IsActorObserved(const AActor* Actor, float ObservationDistance, const TArray<FSpawnViewer>& Viewers) const
{
	FVector Origin;
	FVector Extent;
	Actor->GetActorBounds(true, Origin, Extent);

	return IsObserved(Origin - FVector(0.f, 0.f, Extent.Z), Extent.Z * 2.f, ObservationDistance, Viewers, Actor);
}

// ---------------------------------------------------------------------------
// Atualização periódica
// ---------------------------------------------------------------------------

void AWorldSpawnDirector::UpdatePopulation()
{
	TArray<FSpawnViewer> Viewers;
	GatherViewers(Viewers);

	if (Viewers.Num() == 0)
	{
		return;
	}

	UpdateDeparting(Viewers);

	if (bIsNight)
	{
		UpdateRecords(MonsterRecords, MonsterRules, true, Viewers);
	}

	UpdateRecords(ItemRecords, ItemRules, false, Viewers);

	if (bDrawDebug)
	{
		DrawDebugRecords();
	}
}

void AWorldSpawnDirector::UpdateRecords(TArray<FSpawnRecord>& Records, const FSpawnStreamingRules& Rules, bool bMonsters, const TArray<FSpawnViewer>& Viewers)
{
	const double Now = GetWorld()->GetTimeSeconds();
	int32 ActiveCount = 0;
	int32 DematerializeBudget = MaxDematerializationsPerUpdate;
	TArray<TPair<float, int32>> Candidates;

	for (int32 Index = 0; Index < Records.Num(); ++Index)
	{
		FSpawnRecord& Record = Records[Index];

		if (Record.State == ESpawnRecordState::Gone)
		{
			continue;
		}

		if (Record.State == ESpawnRecordState::Physical)
		{
			AActor* Actor = Record.Actor.Get();
			if (!IsValid(Actor))
			{
				Record.State = ESpawnRecordState::Gone;
				continue;
			}

			// Só some quando ninguém está por perto e ninguém está olhando
			const float Distance = DistanceToNearestViewer2D(FVector2D(Actor->GetActorLocation()), Viewers);
			if (Distance > Rules.DematerializeDistance && DematerializeBudget > 0 && !IsActorObserved(Actor, Rules.ObservationDistance, Viewers))
			{
				DematerializeRecord(Record, bMonsters);
				--DematerializeBudget;
				continue;
			}

			++ActiveCount;
			continue;
		}

		if (Now < Record.ActivationTime)
		{
			continue;
		}

		const float Distance = DistanceToNearestViewer2D(Record.Position, Viewers);
		if (Distance <= Rules.MaterializeDistance && Distance >= Rules.MinDistanceFromPlayers)
		{
			Candidates.Emplace(Distance, Index);
		}
	}

	Candidates.Sort([](const TPair<float, int32>& A, const TPair<float, int32>& B) { return A.Key < B.Key; });

	int32 SpawnBudget = MaxMaterializationsPerUpdate;
	int32 CheckBudget = MaxMaterializationsPerUpdate * 6;

	for (const TPair<float, int32>& Candidate : Candidates)
	{
		if (SpawnBudget <= 0 || CheckBudget-- <= 0 || (Rules.MaxActive > 0 && ActiveCount >= Rules.MaxActive))
		{
			break;
		}

		FSpawnRecord& Record = Records[Candidate.Value];

		if (!Record.bGroundResolved)
		{
			FVector Ground;
			EGroundResult Result = ResolveGround(Record.Position, bMonsters, Ground);

			if (Result == EGroundResult::NotLoaded)
			{
				// Longe: o chão só não carregou ainda. Perto: é um buraco/fora do mapa
				if (Candidate.Key > GuaranteedLoadedRadius)
				{
					continue;
				}
				Result = EGroundResult::Invalid;
			}

			if (Result == EGroundResult::Invalid)
			{
				RelocateRecord(Record);
				continue;
			}

			Record.GroundLocation = Ground;
			Record.bGroundResolved = true;
		}

		// Nunca nasce na frente de um jogador
		const float Height = bMonsters ? 200.f * Record.Scale : 60.f;
		if (IsObserved(Record.GroundLocation, Height, Rules.ObservationDistance, Viewers, nullptr))
		{
			continue;
		}

		AActor* Spawned = bMonsters ? SpawnMonster(Record) : SpawnItem(Record);
		--SpawnBudget;

		if (Spawned)
		{
			Record.Actor = Spawned;
			Record.State = ESpawnRecordState::Physical;
			++ActiveCount;
		}
		else
		{
			Record.State = ESpawnRecordState::Gone;
		}
	}
}

void AWorldSpawnDirector::UpdateDeparting(const TArray<FSpawnViewer>& Viewers)
{
	const double Now = GetWorld()->GetTimeSeconds();

	for (int32 Index = DepartingActors.Num() - 1; Index >= 0; --Index)
	{
		const FDepartingActor& Departing = DepartingActors[Index];
		AActor* Actor = Departing.Actor.Get();

		if (!IsValid(Actor))
		{
			DepartingActors.RemoveAtSwap(Index);
			continue;
		}

		USpawnPresenceComponent* Presence = Actor->FindComponentByClass<USpawnPresenceComponent>();
		if (Presence && Presence->IsVanishing())
		{
			continue;
		}

		const float ObservationDistance = Departing.bIsMonster ? MonsterRules.ObservationDistance : ItemRules.ObservationDistance;

		if (!IsActorObserved(Actor, ObservationDistance, Viewers))
		{
			Actor->Destroy();
			DepartingActors.RemoveAtSwap(Index);
		}
		else if (Departing.Deadline >= 0.0 && Now > Departing.Deadline)
		{
			EnsurePresence(Actor)->PlayVanishAndDestroy();
		}
	}
}

void AWorldSpawnDirector::DrawDebugRecords() const
{
	const float Lifetime = UpdateInterval + 0.05f;

	for (const FSpawnRecord& Record : MonsterRecords)
	{
		if (Record.State == ESpawnRecordState::Virtual)
		{
			const float Z = Record.bGroundResolved ? Record.GroundLocation.Z : SpawnArea->GetComponentLocation().Z;
			DrawDebugPoint(GetWorld(), FVector(Record.Position, Z + 100.f), 12.f, FColor::Red, false, Lifetime);
		}
	}

	for (const FSpawnRecord& Record : ItemRecords)
	{
		if (Record.State == ESpawnRecordState::Virtual)
		{
			const float Z = Record.bGroundResolved ? Record.GroundLocation.Z : SpawnArea->GetComponentLocation().Z;
			DrawDebugPoint(GetWorld(), FVector(Record.Position, Z + 100.f), 10.f, FColor::Green, false, Lifetime);
		}
	}
}

// ---------------------------------------------------------------------------
// Criação e remoção de atores
// ---------------------------------------------------------------------------

AActor* AWorldSpawnDirector::SpawnMonster(const FSpawnRecord& Record)
{
	if (!MonsterTable || !MonsterTable->Entries.IsValidIndex(Record.EntryIndex))
	{
		return nullptr;
	}

	const TSubclassOf<AEnemyCharacter> MonsterClass = MonsterTable->Entries[Record.EntryIndex].MonsterClass;
	if (!MonsterClass)
	{
		return nullptr;
	}

	float CapsuleHalfHeight = 90.f;
	if (const ACharacter* DefaultCharacter = MonsterClass->GetDefaultObject<ACharacter>())
	{
		if (const UCapsuleComponent* Capsule = DefaultCharacter->GetCapsuleComponent())
		{
			CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		}
	}

	const FVector Location = Record.GroundLocation + FVector(0.f, 0.f, CapsuleHalfHeight * Record.Scale + 10.f);
	const FTransform SpawnTransform(FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f), Location, FVector(Record.Scale));

	AEnemyCharacter* Monster = GetWorld()->SpawnActorDeferred<AEnemyCharacter>(MonsterClass, SpawnTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	if (!Monster)
	{
		return nullptr;
	}

	if (Record.bOverridePower && Monster->EnemyStats)
	{
		Monster->EnemyStats->EnemyPower = Record.Power;
	}

	Monster->FinishSpawning(SpawnTransform);

	if (!IsValid(Monster))
	{
		return nullptr;
	}

	// HP guardado de quando ele voltou a ser ficha (tomou dano e o jogador se afastou)
	if (Record.SavedHP >= 0 && Monster->EnemyStats)
	{
		Monster->EnemyStats->HP = Record.SavedHP;
	}

	if (!Monster->GetController())
	{
		Monster->SpawnDefaultController();
	}

	TrackActor(Monster);
	EnsurePresence(Monster)->PlayAppear();

	return Monster;
}

AActor* AWorldSpawnDirector::SpawnItem(const FSpawnRecord& Record)
{
	if (!ItemTable || !ItemTable->Entries.IsValidIndex(Record.EntryIndex))
	{
		return nullptr;
	}

	const FItemSpawnEntry& Entry = ItemTable->Entries[Record.EntryIndex];
	if (!Entry.ItemClass)
	{
		return nullptr;
	}

	// O AItem faz SnapToGround no BeginPlay, então basta nascer um pouco acima do chão
	const FTransform SpawnTransform(FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f), Record.GroundLocation + FVector(0.f, 0.f, 50.f));

	AItem* Item = GetWorld()->SpawnActorDeferred<AItem>(Entry.ItemClass, SpawnTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!Item)
	{
		return nullptr;
	}

	if (Entry.ItemDataOverride)
	{
		Item->ItemData = Entry.ItemDataOverride;
	}
	Item->Quantity = Record.Quantity;

	Item->FinishSpawning(SpawnTransform);

	if (!IsValid(Item))
	{
		return nullptr;
	}

	TrackActor(Item);
	EnsurePresence(Item)->PlayAppear();

	return Item;
}

void AWorldSpawnDirector::DematerializeRecord(FSpawnRecord& Record, bool bMonsters)
{
	AActor* Actor = Record.Actor.Get();
	Record.Actor.Reset();
	Record.State = ESpawnRecordState::Virtual;

	if (!Actor)
	{
		return;
	}

	if (bMonsters)
	{
		// Monstros andam: guarda onde ele estava e quanto de vida tinha
		Record.Position = FVector2D(Actor->GetActorLocation());
		Record.bGroundResolved = false;

		if (const AEnemyCharacter* Monster = Cast<AEnemyCharacter>(Actor))
		{
			if (Monster->EnemyStats)
			{
				Record.SavedHP = Monster->EnemyStats->HP;

				if (Record.SavedHP <= 0)
				{
					Record.State = ESpawnRecordState::Gone;
				}
			}
		}
	}

	ReleaseActor(Actor);
	Actor->Destroy();
}

void AWorldSpawnDirector::SendAway(AActor* Actor, bool bIsMonster, const TArray<FSpawnViewer>& Viewers)
{
	ReleaseActor(Actor);

	const float ObservationDistance = bIsMonster ? MonsterRules.ObservationDistance : ItemRules.ObservationDistance;

	// Ninguém vendo: some na hora, ninguém percebe
	if (!IsActorObserved(Actor, ObservationDistance, Viewers))
	{
		Actor->Destroy();
		return;
	}

	FDepartingActor& Departing = DepartingActors.AddDefaulted_GetRef();
	Departing.Actor = Actor;
	Departing.bIsMonster = bIsMonster;

	if (bIsMonster)
	{
		// Alguém vendo: o monstro foge do jogador e some quando sair da vista
		Departing.Deadline = GetWorld()->GetTimeSeconds() + RetreatTimeout;

		if (AEnemyCharacter* Monster = Cast<AEnemyCharacter>(Actor))
		{
			StartRetreat(Monster, Viewers);
		}
	}
	// Itens sendo vistos ficam até o jogador olhar para outro lado (ou pegar)
}

void AWorldSpawnDirector::StartRetreat(AEnemyCharacter* Monster, const TArray<FSpawnViewer>& Viewers) const
{
	AAIController* AIController = Cast<AAIController>(Monster->GetController());
	if (!AIController)
	{
		return;
	}

	if (UBrainComponent* Brain = AIController->GetBrainComponent())
	{
		Brain->StopLogic(TEXT("Dawn"));
	}

	const FVector MonsterLocation = Monster->GetActorLocation();
	FVector NearestPlayer = MonsterLocation;
	float NearestDistance = TNumericLimits<float>::Max();

	for (const FSpawnViewer& Viewer : Viewers)
	{
		const float Distance = FVector::DistSquared(Viewer.PawnLocation, MonsterLocation);
		if (Distance < NearestDistance)
		{
			NearestDistance = Distance;
			NearestPlayer = Viewer.PawnLocation;
		}
	}

	FVector AwayDirection = (MonsterLocation - NearestPlayer).GetSafeNormal2D();
	if (AwayDirection.IsNearlyZero())
	{
		AwayDirection = FVector(FMath::VRand().GetSafeNormal2D());
	}

	FVector Destination = MonsterLocation + AwayDirection * RetreatDistance;

	if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
	{
		FNavLocation NavLocation;
		if (NavSys->GetRandomReachablePointInRadius(Destination, 1500.f, NavLocation))
		{
			Destination = NavLocation.Location;
		}
	}

	Monster->SetMovementSpeed(true);
	AIController->MoveToLocation(Destination, 100.f);
}

void AWorldSpawnDirector::TrackActor(AActor* Actor)
{
	Actor->OnDestroyed.AddUniqueDynamic(this, &AWorldSpawnDirector::HandleSpawnedActorDestroyed);
}

void AWorldSpawnDirector::ReleaseActor(AActor* Actor)
{
	Actor->OnDestroyed.RemoveDynamic(this, &AWorldSpawnDirector::HandleSpawnedActorDestroyed);
}

USpawnPresenceComponent* AWorldSpawnDirector::EnsurePresence(AActor* Actor) const
{
	if (USpawnPresenceComponent* Existing = Actor->FindComponentByClass<USpawnPresenceComponent>())
	{
		return Existing;
	}

	return Cast<USpawnPresenceComponent>(Actor->AddComponentByClass(USpawnPresenceComponent::StaticClass(), false, FTransform::Identity, false));
}

// Destruído por fora do diretor = monstro morto ou item coletado: não volta mais
void AWorldSpawnDirector::HandleSpawnedActorDestroyed(AActor* DestroyedActor)
{
	for (TArray<FSpawnRecord>* Records : { &MonsterRecords, &ItemRecords })
	{
		for (FSpawnRecord& Record : *Records)
		{
			if (Record.State == ESpawnRecordState::Physical && Record.Actor == DestroyedActor)
			{
				Record.State = ESpawnRecordState::Gone;
				Record.Actor.Reset();
				return;
			}
		}
	}
}
