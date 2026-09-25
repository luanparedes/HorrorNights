// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "TimeManager.generated.h"

UCLASS()
class HORRORNIGHTS_API ATimeManager : public AActor
{
	GENERATED_BODY()
	
public:
	ATimeManager();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

private:	
	UPROPERTY(EditAnywhere, Category = "Time Manager")
	ADirectionalLight* SunLight;

	UPROPERTY(EditAnywhere, Category = "Time Manager")
	ADirectionalLight* MoonLight;

	UPROPERTY(EditAnywhere, Category = "Time Manager")
	ASkyLight* SkyLight;
};
