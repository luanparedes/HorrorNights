// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerClassData.h"
#include "PlayerStatsComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HORRORNIGHTS_API UPlayerStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPlayerStatsComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Class")
	UPlayerClassData* CurrentClassData;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Status")
	int32 CurrentHP;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Status")
	int32 CurrentEnergy;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
	TArray<FName> CurrentInventory;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Habilities")
	TArray<FName> CurrentHabilities;

	UFUNCTION(BlueprintCallable, Category = "Status")
	void InitializeStats();

protected:
	virtual void BeginPlay() override;		
};
