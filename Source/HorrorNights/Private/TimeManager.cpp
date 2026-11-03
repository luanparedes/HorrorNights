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

	// Lua começa apagada, só aparece com o fade da noite
	if (MoonLight)
	{
		MoonLight->GetLightComponent()->SetIntensity(0.f);
		MoonLight->GetLightComponent()->SetVisibility(false);
		RefreshSunLight();
	}
}

void ATimeManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MoonLight)
	{
		ULightComponent* Moon = MoonLight->GetLightComponent();
		const float Speed = MoonMaxIntensity / FMath::Max(FadeDuration, KINDA_SMALL_NUMBER);
		const float NewIntensity = FMath::FInterpConstantTo(Moon->Intensity, MoonTargetIntensity, DeltaTime, Speed);

		if (!FMath::IsNearlyEqual(NewIntensity, Moon->Intensity))
		{
			Moon->SetIntensity(NewIntensity);
		}

		// Ao terminar o fade-out, desliga de vez para não influenciar o dia
		if (NewIntensity <= KINDA_SMALL_NUMBER && MoonTargetIntensity <= 0.f && Moon->IsVisible())
		{
			Moon->SetVisibility(false);
			RefreshSunLight();
		}
	}
}

void ATimeManager::RefreshSunLight()
{
	// Com Forward Shading só existe uma luz direcional principal; ao esconder a lua
	// o slot fica vazio e o sol só volta a ocupá-lo quando é registrado de novo
	if (SunLight)
	{
		SunLight->GetLightComponent()->MarkRenderStateDirty();
	}
}

void ATimeManager::StartNight()
{
	ShowChangeTimeMessage(TEXT("It's night! Be careful!"));

	//if (SunLight)
	//{
	//	SunLight->GetLightComponent()->SetVisibility(false);
	//}
	if (MoonLight)
	{
		ULightComponent* Moon = MoonLight->GetLightComponent();
		if (!Moon->IsVisible())
		{
			Moon->SetIntensity(0.f);
			Moon->SetVisibility(true);
		}
		MoonTargetIntensity = MoonMaxIntensity;
	}
	//if (SkyLight)
	//{
	//	SkyLight->GetLightComponent()->SetVisibility(false);
	//	SkyLight->GetLightComponent()->SetIntensity(0.2f);
	//}
	if (EnemySpawner)
	{
		EnemySpawner->BeginNight();
	}
}

void ATimeManager::StartDay()
{
	ShowChangeTimeMessage(TEXT("Day has begun!"));

	//if (SunLight)
	//{
	//	SunLight->GetLightComponent()->SetVisibility(true);
	//}
	MoonTargetIntensity = 0.f;
	//if (SkyLight)
	//{
	//	SkyLight->GetLightComponent()->SetVisibility(true);
	//	SkyLight->GetLightComponent()->SetIntensity(0.4f);
	//	SkyLight->GetLightComponent()->RecaptureSky();
	//}
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