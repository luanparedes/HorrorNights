// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HorrorNightGameMode.generated.h"

class UPlayerClassData;

UCLASS()
class HORRORNIGHTS_API AHorrorNightGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Character Classes")
	TArray<UPlayerClassData*> AvailableClasses;

	virtual void BeginPlay() override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

protected:
	void SetupHighEndRendering();
	void SetupFallbackRendering();

private:
	void ConfigureGraphics();
};
