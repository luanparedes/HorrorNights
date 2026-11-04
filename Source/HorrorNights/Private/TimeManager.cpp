// Sunnymoon Softwares - All rights reserved


#include "TimeManager.h"
#include "Components/LightComponent.h"
#include "Components/SkyLightComponent.h"

ATimeManager::ATimeManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATimeManager::BeginPlay()
{
	Super::BeginPlay();	

	if (GoodSkyActor)
	{
		FScriptDelegate StartNightDelegate;
		FScriptDelegate StartDayDelegate;
		StartNightDelegate.BindUFunction(this, "StartNight");
		StartDayDelegate.BindUFunction(this, "StartDay");

		if (FMulticastDelegateProperty* DelegateProperty = FindFProperty<FMulticastDelegateProperty>(GoodSkyActor->GetClass(), FName("NightBeginEvent")))
		{
			if (FMulticastScriptDelegate* MulticastDelegate = DelegateProperty->ContainerPtrToValuePtr<FMulticastScriptDelegate>(GoodSkyActor))
			{
				MulticastDelegate->AddUnique(StartNightDelegate);
			}
		}

		if (FMulticastDelegateProperty* DelegateProperty = FindFProperty<FMulticastDelegateProperty>(GoodSkyActor->GetClass(), FName("DayBeginEvent")))
		{
			if (FMulticastScriptDelegate* MulticastDelegate = DelegateProperty->ContainerPtrToValuePtr<FMulticastScriptDelegate>(GoodSkyActor))
			{
				MulticastDelegate->AddUnique(StartDayDelegate);
			}
		}
	}

	// A noite agora é feita só pelo SkyLight; a lua fica desligada.
	// Com Forward Shading só uma luz direcional é a principal, então o sol é
	// registrado de novo para reassumir o lugar que a lua ocupava.
	if (MoonLight)
	{
		MoonLight->GetLightComponent()->SetVisibility(false);

		if (SunLight)
		{
			SunLight->GetLightComponent()->MarkRenderStateDirty();
		}
	}

	UpdateSkyLight();
}

void ATimeManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateSkyLight();
}

void ATimeManager::UpdateSkyLight()
{
	if (!SunLight || !SkyLight)
	{
		return;
	}

	// Direção da luz aponta para o chão; invertendo temos a altura do sol no céu
	const float SunHeight = -SunLight->GetActorForwardVector().Z;
	const float DayAlpha = FMath::SmoothStep(NightSunHeight, DaySunHeight, SunHeight);

	if (FMath::IsNearlyEqual(DayAlpha, LastDayAlpha, 0.001f))
	{
		return;
	}
	LastDayAlpha = DayAlpha;

	// Sem RecaptureSky: a captura do dia é só escurecida e azulada,
	// assim a noite fica bem escura mas ainda dá para enxergar
	USkyLightComponent* Sky = SkyLight->GetLightComponent();
	Sky->SetIntensity(FMath::Lerp(NightSkyIntensity, DaySkyIntensity, DayAlpha));
	Sky->SetLightColor(FMath::Lerp(NightSkyColor, DaySkyColor, DayAlpha));
}

void ATimeManager::StartNight()
{
	ShowChangeTimeMessage(TEXT("It's night! Be careful!"));

	if (SunLight)
	{
		//SunLight->GetLightComponent()->SetVisibility(false);
	}
	if (MoonLight)
	{
		//MoonLight->GetLightComponent()->SetVisibility(true);
	}
	if (EnemySpawner)
	{
		EnemySpawner->BeginNight();
	}
}

void ATimeManager::StartDay()
{
	ShowChangeTimeMessage(TEXT("Day has begun!"));

	if (SunLight)
	{
		//SunLight->GetLightComponent()->SetVisibility(true);
	}
	if (MoonLight)
	{
		//MoonLight->GetLightComponent()->SetVisibility(false);
	}
	if (EnemySpawner)
	{
		EnemySpawner->BeginDay();
	}
}

void ATimeManager::ShowChangeTimeMessage(const FString& Message)
{
	if (!ChangeTimeMessageWidget)
	{
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			ChangeTimeMessageWidget = CreateWidget<UChangeTimeMessageWidget>(PC, MessageWidgetClass);

			if (ChangeTimeMessageWidget)
			{
				ChangeTimeMessageWidget->AddToViewport();
			}
		}
	}

	if (ChangeTimeMessageWidget)
	{
		ChangeTimeMessageWidget->SetMessageText(Message);
	}
}