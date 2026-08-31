// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HorrorNightGameMode.generated.h"

UCLASS()
class HORRORNIGHTS_API AHorrorNightGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	void BeginPlay() override;

	/*UFUNCTION(BlueprintCallable)
	void HostLanGame();
	
	UFUNCTION(BlueprintCallable)
	void JoinLanGame();*/

private:
	void ConfigureGraphics();
};
