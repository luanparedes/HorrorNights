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
		if (UserSettings->GetVisualEffectQuality() == 0)
		{
			//TODO: Deactivate some actors or components to improve performance
		}
		UserSettings->RunHardwareBenchmark();
		UserSettings->ApplyHardwareBenchmarkResults();
		UserSettings->ApplySettings(false);
	}
}