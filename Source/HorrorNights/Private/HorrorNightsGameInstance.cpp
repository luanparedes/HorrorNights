// Sunnymoon Softwares - All rights reserved


#include "HorrorNightsGameInstance.h"
#include "GameFramework/GameUserSettings.h"

void UHorrorNightsGameInstance::Init() 
{
	Super::Init();
	ConfigureGraphics();
}

void UHorrorNightsGameInstance::ConfigureGraphics() 
{
	UGameUserSettings* UserSettings = GEngine->GetGameUserSettings();

	if (UserSettings)
	{
        UserSettings->SetOverallScalabilityLevel(1);
        UserSettings->ApplySettings(true);
	}
}