// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LevelPreloadSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLevelPreloaded);

/**
 * Carrega um mapa em segundo plano para que o OpenLevel seja instantâneo depois.
 * Usado pela intro para pré-carregar o menu principal enquanto a animação toca.
 */
UCLASS()
class HORRORNIGHTS_API ULevelPreloadSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Começa a carregar o mapa em segundo plano
	UFUNCTION(BlueprintCallable, Category = "Level Preload")
	void PreloadLevel(TSoftObjectPtr<UWorld> Level);

	// True quando o preload terminou (com sucesso ou não: o OpenLevel funciona nos dois casos)
	UFUNCTION(BlueprintPure, Category = "Level Preload")
	bool IsLevelPreloaded() const { return bPreloadFinished; }

	// Abre o mapa pré-carregado
	UFUNCTION(BlueprintCallable, Category = "Level Preload")
	void OpenPreloadedLevel();

	// Disparado quando o preload termina
	UPROPERTY(BlueprintAssignable, Category = "Level Preload")
	FOnLevelPreloaded OnLevelPreloaded;

private:
	void OnPackageLoaded(const FName& PackageName, UPackage* LoadedPackage, EAsyncLoadingResult::Type Result);
	void OnPostLoadMap(UWorld* LoadedWorld);

	TSoftObjectPtr<UWorld> PendingLevel;

	// Segura o mundo pré-carregado para o GC não descartar antes do OpenLevel
	UPROPERTY(Transient)
	TObjectPtr<UWorld> PreloadedWorld;

	bool bPreloading = false;
	bool bPreloadFinished = false;
	FDelegateHandle PostLoadMapHandle;
};
