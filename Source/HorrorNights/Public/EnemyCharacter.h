// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "EnemyStatsComponent.h"
#include "EnemyClassData.h"
#include "EnemyCharacter.generated.h"

UCLASS()
class HORRORNIGHTS_API AEnemyCharacter : public ABaseCharacter
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Character Stats")
	UEnemyStatsComponent* EnemyStats;

	UPROPERTY(EditAnywhere, Category = "Spawn Configuration")
	TArray<AActor*> SpawnPoints;

	AEnemyCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void Attack() override;
	virtual void Defense() override;
	virtual void Magic() override;
	virtual void Ability() override;

	UFUNCTION(BlueprintCallable, Category = "AI Movement")
	void SetMovementSpeed(bool bIsRunning);

protected:
	virtual void BeginPlay() override;

private:
	float Multiplier = 1.0f;
};
