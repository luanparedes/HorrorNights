// Sunnymoon Softwares - All rights reserved


#include "IntroGameMode.h"
#include "IntroWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

AIntroGameMode::AIntroGameMode()
{
	// Sem personagem: o IntroLevel precisa carregar o mais rápido possível
	DefaultPawnClass = nullptr;
}

void AIntroGameMode::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC || !IntroWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("IntroGameMode: defina o IntroWidgetClass no Blueprint do GameMode."));
		return;
	}

	if (UIntroWidget* Widget = CreateWidget<UIntroWidget>(PC, IntroWidgetClass))
	{
		Widget->AddToViewport();
	}
}
