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
}

void ATimeManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
	if (SkyLight)
	{
		//SkyLight->GetLightComponent()->SetVisibility(false);
		SkyLight->GetLightComponent()->SetIntensity(0.02f);
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
	if (SkyLight)
	{
		//SkyLight->GetLightComponent()->SetVisibility(true);
		SkyLight->GetLightComponent()->SetIntensity(0.4f);
		SkyLight->GetLightComponent()->RecaptureSky();
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