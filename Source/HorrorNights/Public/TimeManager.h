// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "EnemyIntelligentSpawner.h"
#include "ChangeTimeMessageWidget.h"
#include "TimeManager.generated.h"

UCLASS()
class HORRORNIGHTS_API ATimeManager : public AActor
{
	GENERATED_BODY()
	
public:
	ATimeManager();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Time Manager")
	void StartNight();
	
	UFUNCTION(BlueprintCallable, Category = "Time Manager")
	void StartDay();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowChangeTimeMessage(const FString& Message);

protected:
	virtual void BeginPlay() override;

	void RefreshSunLight();

private:
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UChangeTimeMessageWidget> MessageWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	UChangeTimeMessageWidget* ChangeTimeMessageWidget;

	UPROPERTY(EditAnywhere, Category = "Time Manager")
	AActor* GoodSkyActor;

	UPROPERTY(EditAnywhere, Category = "Time Manager")
	ADirectionalLight* SunLight;

	UPROPERTY(EditAnywhere, Category = "Time Manager")
	ADirectionalLight* MoonLight;

	UPROPERTY(EditAnywhere, Category = "Time Manager")
	ASkyLight* SkyLight;

	UPROPERTY(EditAnywhere, Category = "Spawn Manager")
	AEnemyIntelligentSpawner* EnemySpawner;

	UPROPERTY(EditAnywhere, Category = "Time Manager|Fade")
	float FadeDuration = 5.f;

	UPROPERTY(EditAnywhere, Category = "Time Manager|Fade")
	float MoonMaxIntensity = 1.f;

	float MoonTargetIntensity = 0.f;
};
