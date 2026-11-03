// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyClassData.h"
#include "EnemyPower.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnemyStatsComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HORRORNIGHTS_API UEnemyStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyStatsComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Class")
	UEnemyClassData* CurrentClassData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
	EEnemyPower EnemyPower = EEnemyPower::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
	int32 HP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
	int32 AttackPower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
	int32 DefensePower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
	int32 MagicAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
	int32 Speed;

	UFUNCTION(BlueprintCallable, Category = "Status")
	void InitializeStats();

protected:
	virtual void BeginPlay() override;

private:
	float GetMultiplierFromPower();
};
