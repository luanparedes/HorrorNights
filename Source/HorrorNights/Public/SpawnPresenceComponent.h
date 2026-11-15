// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpawnPresenceComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSpawnPresenceChanged);

// Faz o ator surgir e sumir aos poucos. O fade usa um parâmetro escalar nos
// materiais (padrão "SpawnFade", 0 = invisível, 1 = visível); materiais sem esse
// parâmetro simplesmente ignoram. Os eventos servem para tocar som, partícula
// ou animação na Blueprint.
UCLASS(ClassGroup = (Spawning), meta = (BlueprintSpawnableComponent))
class HORRORNIGHTS_API USpawnPresenceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpawnPresenceComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Presence")
	FName FadeParameterName = TEXT("SpawnFade");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Presence", meta = (ClampMin = "0"))
	float FadeInDuration = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Presence", meta = (ClampMin = "0"))
	float FadeOutDuration = 2.f;

	UPROPERTY(BlueprintAssignable, Category = "Spawn Presence")
	FOnSpawnPresenceChanged OnAppearStarted;

	UPROPERTY(BlueprintAssignable, Category = "Spawn Presence")
	FOnSpawnPresenceChanged OnVanishStarted;

	UFUNCTION(BlueprintCallable, Category = "Spawn Presence")
	void PlayAppear();

	UFUNCTION(BlueprintCallable, Category = "Spawn Presence")
	void PlayVanishAndDestroy();

	UFUNCTION(BlueprintPure, Category = "Spawn Presence")
	bool IsVanishing() const { return bVanishing; }

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void ApplyFade(float Alpha) const;

	float FadeAlpha = 1.f;
	float FadeTarget = 1.f;
	float FadeSpeed = 1.f;
	bool bVanishing = false;
};
