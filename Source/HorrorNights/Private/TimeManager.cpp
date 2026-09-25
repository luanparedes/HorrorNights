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
}

void ATimeManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (SunLight && MoonLight && SkyLight)
	{
		ULightComponent* SunLightComponent = SunLight->GetLightComponent();
		ULightComponent* MoonLightComponent = MoonLight->GetLightComponent();
		USkyLightComponent* SkyLightComponent = SkyLight->GetLightComponent();

		if (SunLightComponent && MoonLightComponent && SkyLightComponent)
		{
			float SunPitch = SunLight->GetActorRotation().Pitch;

			FVector2D RangeTransition(-15.0f, 5.0f);

			// Mapeamos a intensidade máxima e mínima de cada luz
			// DICA: Ajuste esses números iniciais (10.0, 2.0) pro que ficar bonito na sua floresta
			FVector2D RangeSun(10.0f, 0.0f);       // Sol vai de 10 até 0
			FVector2D RangeMoon(0.0f, 2.0f);       // Lua vai de 0 até 2
			FVector2D RangeSky(1.0f, 0.02f);       // SkyLight vai de 1 (dia) até 0.02 (breu da noite)

			// Calcula a intensidade exata para esse exato frame de forma suave
			float NewIntensitySun = FMath::GetMappedRangeValueClamped(RangeTransition, RangeSun, SunPitch);
			float NewIntensityMoon = FMath::GetMappedRangeValueClamped(RangeTransition, RangeMoon, SunPitch);
			float NewIntensitySky = FMath::GetMappedRangeValueClamped(RangeTransition, RangeSky, SunPitch);

			// Aplica as intensidades
			SunLightComponent->SetIntensity(NewIntensitySun);
			MoonLightComponent->SetIntensity(NewIntensityMoon);
			SkyLightComponent->SetIntensity(NewIntensitySky);

			// OTIMIZAÇÃO MOBILE (Onde a mágica do FPS acontece):
			// Se a luz zerou, a gente corta a visibilidade pra GPU não calcular sombras à toa.
			// Se for maior que zero, a gente liga de volta.
			SunLightComponent->SetVisibility(NewIntensitySun > 0.0f);
			MoonLightComponent->SetVisibility(NewIntensityMoon > 0.0f);

			// A SkyLight nunca zera (fica no mínimo 0.02), então garantimos ela sempre visível
			SkyLightComponent->SetVisibility(true);
		}
	}
}

