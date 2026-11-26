// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Ticker.h"
#include "ScreenFadeSubsystem.generated.h"

class SWidget;

/**
 * Fade para/do preto por cima de TUDO (cena e widgets UMG).
 * Também pode clarear a tela sozinho depois que o próximo mapa carregar.
 */
UCLASS()
class HORRORNIGHTS_API UScreenFadeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Escurece a tela até o preto e chama OnFinished no final
	void FadeToBlack(float Duration, TFunction<void()> OnFinished = nullptr);

	// Começa preto e clareia até sumir
	UFUNCTION(BlueprintCallable, Category = "Screen Fade")
	void FadeFromBlack(float Duration);

	// O próximo mapa que carregar começa preto e clareia nessa duração
	UFUNCTION(BlueprintCallable, Category = "Screen Fade")
	void FadeInOnNextMap(float Duration) { NextMapFadeInDuration = Duration; }

private:
	void StartFade(float FromAlpha, float ToAlpha, float Duration, TFunction<void()> OnFinished);
	bool TickFade(float DeltaTime);
	void ShowOverlay();
	void HideOverlay();
	void OnPostLoadMap(UWorld* LoadedWorld);

	TSharedPtr<SWidget> Overlay;
	FTSTicker::FDelegateHandle TickerHandle;
	FDelegateHandle PostLoadMapHandle;
	TFunction<void()> FadeFinished;

	float CurrentAlpha = 0.f;
	float StartAlpha = 0.f;
	float TargetAlpha = 0.f;
	float FadeDuration = 0.f;
	float FadeElapsed = 0.f;
	float NextMapFadeInDuration = 0.f;
};
