// Sunnymoon Softwares - All rights reserved


#include "ScreenFadeSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Styling/CoreStyle.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/Images/SImage.h"

namespace
{
	// Bem acima de qualquer widget do jogo
	constexpr int32 FadeZOrder = 10000;
}

void UScreenFadeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UScreenFadeSubsystem::OnPostLoadMap);
}

void UScreenFadeSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
	FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
	TickerHandle.Reset();
	HideOverlay();
	Super::Deinitialize();
}

void UScreenFadeSubsystem::FadeToBlack(float Duration, TFunction<void()> OnFinished)
{
	StartFade(CurrentAlpha, 1.f, Duration, MoveTemp(OnFinished));
}

void UScreenFadeSubsystem::FadeFromBlack(float Duration)
{
	StartFade(1.f, 0.f, Duration, nullptr);
}

void UScreenFadeSubsystem::StartFade(float FromAlpha, float ToAlpha, float Duration, TFunction<void()> OnFinished)
{
	StartAlpha = FromAlpha;
	TargetAlpha = ToAlpha;
	CurrentAlpha = FromAlpha;
	FadeDuration = FMath::Max(Duration, 0.f);
	FadeElapsed = 0.f;
	FadeFinished = MoveTemp(OnFinished);

	ShowOverlay();

	if (!TickerHandle.IsValid())
	{
		TickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UScreenFadeSubsystem::TickFade));
	}
}

bool UScreenFadeSubsystem::TickFade(float DeltaTime)
{
	// Limita o delta: um frame travado (ex: logo após carregar o mapa) não pode "pular" o fade
	FadeElapsed += FMath::Min(DeltaTime, 1.f / 30.f);
	const float Alpha = FadeDuration > 0.f ? FMath::Clamp(FadeElapsed / FadeDuration, 0.f, 1.f) : 1.f;
	CurrentAlpha = FMath::InterpEaseInOut(StartAlpha, TargetAlpha, Alpha, 2.f);

	if (Alpha < 1.f)
	{
		return true;
	}

	// Fade terminou
	TickerHandle.Reset();
	if (TargetAlpha <= 0.f)
	{
		HideOverlay();
	}

	if (FadeFinished)
	{
		TFunction<void()> Callback = MoveTemp(FadeFinished);
		FadeFinished = nullptr;
		Callback();
	}

	return false;
}

void UScreenFadeSubsystem::ShowOverlay()
{
	UGameViewportClient* Viewport = GetGameInstance() ? GetGameInstance()->GetGameViewportClient() : nullptr;
	if (!Viewport)
	{
		return;
	}

	if (!Overlay.IsValid())
	{
		TWeakObjectPtr<UScreenFadeSubsystem> WeakThis(this);
		Overlay = SNew(SImage)
			.Image(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.Visibility(EVisibility::HitTestInvisible)
			.ColorAndOpacity_Lambda([WeakThis]()
			{
				const float A = WeakThis.IsValid() ? WeakThis->CurrentAlpha : 0.f;
				return FSlateColor(FLinearColor(0.f, 0.f, 0.f, A));
			});
	}

	// O LoadMap remove todos os widgets da viewport, então re-adiciona se precisar
	Viewport->RemoveViewportWidgetContent(Overlay.ToSharedRef());
	Viewport->AddViewportWidgetContent(Overlay.ToSharedRef(), FadeZOrder);
}

void UScreenFadeSubsystem::HideOverlay()
{
	if (Overlay.IsValid())
	{
		if (UGameViewportClient* Viewport = GetGameInstance() ? GetGameInstance()->GetGameViewportClient() : nullptr)
		{
			Viewport->RemoveViewportWidgetContent(Overlay.ToSharedRef());
		}
		Overlay.Reset();
	}
}

void UScreenFadeSubsystem::OnPostLoadMap(UWorld* LoadedWorld)
{
	if (NextMapFadeInDuration > 0.f)
	{
		const float Duration = NextMapFadeInDuration;
		NextMapFadeInDuration = 0.f;
		FadeFromBlack(Duration);
	}
	else if (CurrentAlpha > 0.f)
	{
		// Mapa novo sem fade pedido: não deixa a tela presa no preto
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
		TickerHandle.Reset();
		CurrentAlpha = 0.f;
		HideOverlay();
	}
}
