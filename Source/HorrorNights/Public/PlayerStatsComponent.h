// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerClassData.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayerStatsComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HORRORNIGHTS_API UPlayerStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPlayerStatsComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Class")
	UPlayerClassData* CurrentClassData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
	int32 HP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
	int32 Energy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
	int32 AttackPower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
	int32 DefensePower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
	int32 Smartness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
	int32 MagicAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
	int32 Speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
	int32 JumpStat;

	UFUNCTION(BlueprintCallable, Category = "Status")
	void InitializeStats();

protected:
	virtual void BeginPlay() override;		
};
