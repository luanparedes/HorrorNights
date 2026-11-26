// Sunnymoon Softwares - All rights reserved


#include "LevelPreloadSubsystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

void ULevelPreloadSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ULevelPreloadSubsystem::OnPostLoadMap);
}

void ULevelPreloadSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
	PreloadedWorld = nullptr;
	Super::Deinitialize();
}

void ULevelPreloadSubsystem::PreloadLevel(TSoftObjectPtr<UWorld> Level)
{
	if (Level.IsNull() || bPreloading)
	{
		return;
	}

	PendingLevel = Level;
	bPreloadFinished = false;
	bPreloading = true;

	LoadPackageAsync(Level.GetLongPackageName(),
		FLoadPackageAsyncDelegate::CreateUObject(this, &ULevelPreloadSubsystem::OnPackageLoaded));
}

void ULevelPreloadSubsystem::OnPackageLoaded(const FName& PackageName, UPackage* LoadedPackage, EAsyncLoadingResult::Type Result)
{
	bPreloading = false;

	if (Result == EAsyncLoadingResult::Succeeded && LoadedPackage)
	{
		PreloadedWorld = UWorld::FindWorldInPackage(LoadedPackage);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Preload de %s falhou; o mapa será carregado normalmente no OpenLevel."), *PackageName.ToString());
	}

	// Mesmo se falhar, libera: o OpenLevel carrega o mapa do jeito normal
	bPreloadFinished = true;
	OnLevelPreloaded.Broadcast();
}

void ULevelPreloadSubsystem::OpenPreloadedLevel()
{
	if (!PendingLevel.IsNull())
	{
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, PendingLevel);
	}
}

void ULevelPreloadSubsystem::OnPostLoadMap(UWorld* LoadedWorld)
{
	// O mapa já abriu: solta a referência para não segurar o mundo na memória
	PreloadedWorld = nullptr;
	bPreloadFinished = false;
}
