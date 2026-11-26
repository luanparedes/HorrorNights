// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "IntroWidget.generated.h"

class UWidgetAnimation;

/**
 * Tela de abertura: toca a animação MiniIntro enquanto o menu principal carrega em segundo plano.
 * Vai para o menu quando a animação termina, ou antes se o jogador apertar qualquer botão
 * depois que o menu já carregou.
 */
UCLASS()
class HORRORNIGHTS_API UIntroWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;

	// Mapa que será aberto depois da intro
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro")
	TSoftObjectPtr<UWorld> MainMenuMap = TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/MainMenu/MainMenu.MainMenu")));

	// Tempo (s) escurecendo a intro antes de trocar de mapa
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro", meta = (ClampMin = "0"))
	float FadeOutDuration = 0.8f;

	// Tempo (s) clareando o menu principal depois que ele abre
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro", meta = (ClampMin = "0"))
	float FadeInDuration = 1.0f;

	// O nome TEM que ser igual ao da animação no Widget Blueprint
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> MiniIntro;

	// Chamado quando o menu terminou de carregar e o jogador já pode pular (ex: mostrar "Pressione qualquer botão")
	UFUNCTION(BlueprintImplementableEvent, Category = "Intro")
	void OnSkipAvailable();

private:
	UFUNCTION()
	void HandleIntroFinished();

	UFUNCTION()
	void HandleMainMenuPreloaded();

	FReply TrySkip();
	void GoToMainMenu();
	class ULevelPreloadSubsystem* GetPreloadSubsystem() const;

	bool bAnimationFinished = false;
	bool bLeaving = false;
};
