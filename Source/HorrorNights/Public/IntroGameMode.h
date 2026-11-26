// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "IntroGameMode.generated.h"

class UIntroWidget;

/**
 * GameMode leve para o IntroLevel: não cria personagem, só mostra o widget da intro.
 */
UCLASS()
class HORRORNIGHTS_API AIntroGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AIntroGameMode();

protected:
	virtual void BeginPlay() override;

	// Widget da intro (ex: WBP_Intro, filho de IntroWidget)
	UPROPERTY(EditDefaultsOnly, Category = "Intro")
	TSubclassOf<UIntroWidget> IntroWidgetClass;
};
