// Sunnymoon Softwares - All rights reserved


#include "HorrorNightGameMode.h"
#include "RHI.h"
#include "RenderQualityInterface.h"
#include "Kismet/GameplayStatics.h"

//void AHorrorNightGameMode::HostLanGame() 
//{
//	GetWorld()->ServerTravel("/Game/Maps/Level_1-1?listen");
//}
//
//void AHorrorNightGameMode::JoinLanGame() 
//{
//	APlayerController* pcPlayer = GetGameInstance()->GetFirstLocalPlayerController();
//	if (pcPlayer)
//	{
//		pcPlayer->ClientTravel("192.168.0.14", ETravelType::TRAVEL_Absolute);
//	}
//}

void AHorrorNightGameMode::BeginPlay()
{
	Super::BeginPlay();
	ConfigureGraphics();
}

void AHorrorNightGameMode::ConfigureGraphics()
{
	const bool bHasSM5 = GMaxRHIFeatureLevel >= ERHIFeatureLevel::SM5;

	if (bHasSM5)
	{
        UE_LOG(LogTemp, Log, TEXT("Rendering: SM5 detected - using advanced pipeline"));
		SetupHighEndRendering();
	}
	else
	{
        UE_LOG(LogTemp, Log, TEXT("Rendering: fallback ES3.1 - using lower mobile pipeline."));
		SetupFallbackRendering();
	}
}

void AHorrorNightGameMode::SetupHighEndRendering()
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithInterface(GetWorld(), URenderQualityInterface::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        IRenderQualityInterface::Execute_OnHighEndRenderingEnabled(Actor);
    }
}

void AHorrorNightGameMode::SetupFallbackRendering()
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithInterface(GetWorld(), URenderQualityInterface::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        IRenderQualityInterface::Execute_OnFallbackRenderingEnabled(Actor);
    }
}