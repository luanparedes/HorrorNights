// Sunnymoon Softwares - All rights reserved


#include "EnemyIntelligentSpawner.h"
#include "PlayerCharacter.h"
#include "EnemyPower.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"

AEnemyIntelligentSpawner::AEnemyIntelligentSpawner()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemyIntelligentSpawner::BeginPlay()
{
	Super::BeginPlay();
}

void AEnemyIntelligentSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEnemyIntelligentSpawner::BeginNight()
{
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("Entrou no BeginNight!"));

	NightWolves.Empty();
	bIsNightTime = true;

	float MinimumDistanceBetweenEnemies = 1000.0f;
	int32 enemyQuantity = FMath::RandRange(200, 400);

	if (!EnemyClass || SpawnAnchors.Num() == 0 || !CabinTargetPoint) 
		return;

	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("Entrando no primeiro FOR!"));

	for (int32 i = 0; i < enemyQuantity; i++)
	{
		int32 RandomIndex = FMath::RandRange(0, SpawnAnchors.Num() - 1);
		AActor* SelectedAnchor = SpawnAnchors[RandomIndex];

		// Cria uma coordenada "chutada" num raio de 2500 perto da âncora
		FVector RandomOffset = FVector(FMath::RandRange(-2500.0f, 2500.0f), FMath::RandRange(-2500.0f, 2500.0f), 0.0f);
		FVector GhostLocation = SelectedAnchor->GetActorLocation() + RandomOffset;

		// Checa se não caiu dentro da cabana
		float CabinDistance = FVector::Dist(GhostLocation, CabinTargetPoint->GetActorLocation());
		if (CabinDistance > MinimumDistanceAwayFromCabin)
		{
			// Cria a ficha direto, sem perguntar pro NavMesh
			FVirtualWolfData NewVirtualWolf;
			NewVirtualWolf.CurrentLocation = GhostLocation;
			NewVirtualWolf.EnemyScale = FMath::FRandRange(0.3f, 1.5f);
			NewVirtualWolf.EnemyPower = static_cast<EEnemyPower>(FMath::RandRange(0, 3));

			NightWolves.Add(NewVirtualWolf);
		}
		else
		{
			// Se caiu na cabana, volta uma casa no loop pra não perder esse lobo da contagem
			i--;
		}
	}
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("Saiu do for com %d lobos"), NightWolves.Num()));

	GetWorldTimerManager().SetTimer(NightTimerHandle, this, &AEnemyIntelligentSpawner::ProcessNightSpawns, 2.0f, true);
	UE_LOG(LogTemp, Warning, TEXT("Lobos criados 2: %d"), NightWolves.Num());
}

void AEnemyIntelligentSpawner::ProcessNightSpawns()
{
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Entrou no ProcessNightSpawns!"));

	if (!bIsNightTime) 
		return;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys) return;

	TArray<AActor*> Players;
	// ATENÇÃO: Garante que "AHorrorNightsCharacter" é exatamente a classe do seu herói! 
	// Se tiver errado, a lista vem vazia e nenhum bicho nasce.
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerCharacter::StaticClass(), Players);
	if (Players.Num() == 0) return;

	float SpawnDistance = 20000.0f;
	float DespawnDistance = 23000.0f;

	for (FVirtualWolfData& WolfData : NightWolves)
	{
		float MinDistanceToPlayer = UE_MAX_FLT;

		for (AActor* Player : Players)
		{
			FVector LocationToCheck;

			// Checa se a ficha diz que ele spawnou E se o corpo físico realmente existe e está ativo na memória
			if (WolfData.bIsSpawned && IsValid(WolfData.PhysicalWolf))
			{
				LocationToCheck = WolfData.PhysicalWolf->GetActorLocation();
			}
			else
			{
				// Se o corpo foi deletado ou perdido, recua para a coordenada salva e arruma o status
				LocationToCheck = WolfData.CurrentLocation;
				WolfData.bIsSpawned = false;
				WolfData.PhysicalWolf = nullptr;
			}

			float Dist = FVector::Dist(LocationToCheck, Player->GetActorLocation());
			if (Dist < MinDistanceToPlayer) 
				MinDistanceToPlayer = Dist;
		}

		// O JOGADOR SE APROXIMOU -> Chão carregado! Pede pro NavMesh achar o ponto seguro
		if (!WolfData.bIsSpawned && MinDistanceToPlayer <= SpawnDistance)
		{
			FNavLocation NavLoc;
			// Procura o chão verde num raio de 1000 da posição original do fantasma
			if (NavSys->GetRandomReachablePointInRadius(WolfData.CurrentLocation, 1000.0f, NavLoc))
			{
				NavLoc.Location.Z += 180.0f; // Empurrãozinho salvador
				FTransform SpawnTransform(FRotator::ZeroRotator, NavLoc.Location);

				AEnemyCharacter* NewWolf = GetWorld()->SpawnActorDeferred<AEnemyCharacter>(EnemyClass, SpawnTransform);

				if (NewWolf)
				{
					NewWolf->SetActorScale3D(FVector(WolfData.EnemyScale));
					NewWolf->EnemyStats->EnemyPower = WolfData.EnemyPower;

					NewWolf->FinishSpawning(SpawnTransform);

					if (WolfData.CurrentHP != -1.0f)
					{
						NewWolf->EnemyStats->HP = WolfData.CurrentHP;
					}

					WolfData.PhysicalWolf = NewWolf;
					WolfData.bIsSpawned = true;
					// Atualiza a ficha com a posição real de onde ele pisou
					WolfData.CurrentLocation = NavLoc.Location;
				}
			}
		}
		// JOGADOR FUGIU -> Tira o corpo físico da reta
		else if (WolfData.bIsSpawned && MinDistanceToPlayer > DespawnDistance)
		{
			if (WolfData.PhysicalWolf)
			{
				WolfData.CurrentLocation = WolfData.PhysicalWolf->GetActorLocation();
				WolfData.CurrentHP = WolfData.PhysicalWolf->EnemyStats->HP;
				WolfData.PhysicalWolf->Destroy();
			}
			WolfData.PhysicalWolf = nullptr;
			WolfData.bIsSpawned = false;
		}
	}
}

void AEnemyIntelligentSpawner::BeginDay()
{
	bIsNightTime = false;
	GetWorldTimerManager().ClearTimer(NightTimerHandle);

	// Deleta os que estão físicos no mapa
	for (FVirtualWolfData& WolfData : NightWolves)
	{
		if (WolfData.bIsSpawned && IsValid(WolfData.PhysicalWolf))
		{
			WolfData.PhysicalWolf->Destroy();
		}
	}

	NightWolves.Empty();
}