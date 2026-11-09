// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "CheatDefinition.h"
#include "CheatStartingItems.generated.h"

class UItemData;

UCLASS(meta = (DisplayName = "Cheat - Starting Items"))
class HORRORNIGHTS_API UCheatStartingItems : public UCheatDefinition
{
	GENERATED_BODY()

public:
	UCheatStartingItems();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cheat|Items")
	TMap<TObjectPtr<UItemData>, int32> Items;

	virtual void ApplyToPlayer_Implementation(APlayerCharacter* Player) override;
};
