// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameFramework/Character.h"
#include "PlayerClassData.generated.h"

UCLASS()
class HORRORNIGHTS_API UPlayerClassData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	TSubclassOf<class ACharacter> CharacterBlueprint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status")
	int32 HP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status")
	int32 Energy;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status")
	int32 Attack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status")
	int32 Defense;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status")
	int32 Smartness;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status")
	int32 MagicAttack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status")
	int32 Speed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status")
	int32 Jump;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status")
	double WaitMagicTimeMultiplier;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status")
	double HealTimeMultiplier;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status")
	double CookingTimeMultiplier;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status")
	double DriveVelocityMultiplier;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	TArray<FName> Inventory;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Special Habilities")
	TArray<FName> Habilities;
};
