// Sunnymoon Softwares - All rights reserved


#include "HorrorNightGameMode.h"

void AHorrorNightGameMode::HostLanGame() 
{
	GetWorld()->ServerTravel("/Game/Maps/Level_1-1?listen");
}

void AHorrorNightGameMode::JoinLanGame() 
{
	APlayerController* pcPlayer = GetGameInstance()->GetFirstLocalPlayerController();
	if (pcPlayer)
	{
		pcPlayer->ClientTravel("192.168.0.14", ETravelType::TRAVEL_Absolute);
	}
}