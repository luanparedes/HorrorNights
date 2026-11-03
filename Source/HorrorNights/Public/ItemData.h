// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayEffect.h"
#include "ItemType.h"
#include "ItemData.generated.h"

UCLASS(BlueprintType)
class HORRORNIGHTS_API UItemData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	UTexture2D* ItemIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> ItemEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	EItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (EditCondition = "ItemType == EItemType::Food", EditConditionHides))
	EFoodItems FoodItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (EditCondition = "ItemType == EItemType::Materials", EditConditionHides))
	EMaterialsItems MaterialItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (EditCondition = "ItemType == EItemType::Recovery", EditConditionHides))
	ERecoveryItems RecoveryItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (EditCondition = "ItemType == EItemType::Upgrade", EditConditionHides))
	EUpgradeItems UpgradeItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (EditCondition = "ItemType == EItemType::KeyItems", EditConditionHides))
	EKeyItems KeyItem;
};