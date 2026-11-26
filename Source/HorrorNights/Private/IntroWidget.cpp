// Sunnymoon Softwares - All rights reserved


#include "IntroWidget.h"
#include "LevelPreloadSubsystem.h"
#include "ScreenFadeSubsystem.h"
#include "Animation/WidgetAnimation.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"

void UIntroWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Recebe teclado/gamepad/mouse/toque
	SetIsFocusable(true);
	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		PC->SetInputMode(InputMode);
	}

	if (MiniIntro)
	{
		FWidgetAnimationDynamicEvent FinishedEvent;
		FinishedEvent.BindDynamic(this, &UIntroWidget::HandleIntroFinished);
		BindToAnimationFinished(MiniIntro, FinishedEvent);
		PlayAnimation(MiniIntro);
	}
	else
	{
		bAnimationFinished = true;
	}

	if (ULevelPreloadSubsystem* Preload = GetPreloadSubsystem())
	{
		Preload->OnLevelPreloaded.AddDynamic(this, &UIntroWidget::HandleMainMenuPreloaded);
		Preload->PreloadLevel(MainMenuMap);
	}
}

void UIntroWidget::NativeDestruct()
{
	if (ULevelPreloadSubsystem* Preload = GetPreloadSubsystem())
	{
		Preload->OnLevelPreloaded.RemoveDynamic(this, &UIntroWidget::HandleMainMenuPreloaded);
	}

	Super::NativeDestruct();
}

void UIntroWidget::HandleIntroFinished()
{
	bAnimationFinished = true;

	const ULevelPreloadSubsystem* Preload = GetPreloadSubsystem();
	if (!Preload || Preload->IsLevelPreloaded())
	{
		GoToMainMenu();
	}
	// Senão, espera o HandleMainMenuPreloaded
}

void UIntroWidget::HandleMainMenuPreloaded()
{
	if (bAnimationFinished)
	{
		GoToMainMenu();
	}
	else
	{
		OnSkipAvailable();
	}
}

FReply UIntroWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	return TrySkip();
}

FReply UIntroWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return TrySkip();
}

FReply UIntroWidget::NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	return TrySkip();
}

FReply UIntroWidget::TrySkip()
{
	const ULevelPreloadSubsystem* Preload = GetPreloadSubsystem();
	if (!Preload || Preload->IsLevelPreloaded())
	{
		GoToMainMenu();
	}

	return FReply::Handled();
}

void UIntroWidget::GoToMainMenu()
{
	if (bLeaving)
	{
		return;
	}
	bLeaving = true;

	ULevelPreloadSubsystem* Preload = GetPreloadSubsystem();
	if (!Preload)
	{
		return;
	}

	UScreenFadeSubsystem* Fade = GetGameInstance() ? GetGameInstance()->GetSubsystem<UScreenFadeSubsystem>() : nullptr;
	if (!Fade)
	{
		Preload->OpenPreloadedLevel();
		return;
	}

	// Escurece a intro, troca de mapa e o menu clareia sozinho quando abrir
	Fade->FadeInOnNextMap(FadeInDuration);
	TWeakObjectPtr<ULevelPreloadSubsystem> WeakPreload(Preload);
	Fade->FadeToBlack(FadeOutDuration, [WeakPreload]()
	{
		if (WeakPreload.IsValid())
		{
			WeakPreload->OpenPreloadedLevel();
		}
	});
}

ULevelPreloadSubsystem* UIntroWidget::GetPreloadSubsystem() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<ULevelPreloadSubsystem>() : nullptr;
}
