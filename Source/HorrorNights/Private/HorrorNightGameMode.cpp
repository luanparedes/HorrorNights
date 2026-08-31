// Sunnymoon Softwares - All rights reserved


#include "HorrorNightGameMode.h"
#include "EngineUtils.h"
#include "Engine/SkyLight.h"
#include "Engine/TextureCube.h"
#include "Components/SkyLightComponent.h"

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
//
//    for (TActorIterator<ASkyLight> It(GetWorld()); It; ++It)
//    {
//        ASkyLight* sky = *It;
//        if (sky && sky->GetLightComponent())
//        {
//#if PLATFORM_ANDROID
//            sky->GetLightComponent()->bRealTimeCapture = false;
//            sky->GetLightComponent()->SourceType = ESkyLightSourceType::SLS_SpecifiedCubemap;
//
//            FString texturePath = TEXT("/Engine/MapTemplates/daylight.daylight");
//            UTextureCube* loadedTexture = LoadObject<UTextureCube>(nullptr, *texturePath);
//
//            if (loadedTexture)
//            {
//                sky->GetLightComponent()->Cubemap = loadedTexture;
//            }
//#else
//            sky->GetLightComponent()->bRealTimeCapture = true;
//            sky->GetLightComponent()->SourceType = ESkyLightSourceType::SLS_CapturedScene;
//#endif
//
//            sky->GetLightComponent()->RecaptureSky();
//        }
//    }
}