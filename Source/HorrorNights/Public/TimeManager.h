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

	UPROPERTY(EditAnywhere, Category = "Time Manager|SkyLight")
	float NightSunHeight = -0.1f;

	UPROPERTY(EditAnywhere, Category = "Time Manager|SkyLight")
	float DaySunHeight = 0.15f;

	UPROPERTY(EditAnywhere, Category = "Time Manager|SkyLight")
	float DaySkyIntensity = 4.f;

	UPROPERTY(EditAnywhere, Category = "Time Manager|SkyLight")
	float NightSkyIntensity = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Time Manager|SkyLight")
	FLinearColor DaySkyColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "Time Manager|SkyLight")
	FLinearColor NightSkyColor = FLinearColor(0.35f, 0.45f, 0.8f);

	float LastDayAlpha = -1.f;

	void UpdateSkyLight();
};
