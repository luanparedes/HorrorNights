// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemData.h"
#include "InventoryComponent.generated.h"

class APlayerCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HORRORNIGHTS_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInventoryComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TMap<UItemData*, int32> InventoryItems;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UseItem(UItemData* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddItem(UItemData* Item, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveItem(UItemData* Item, int32 Quantity = 1);

	// A cereja do bolo: essa função filtra o inventário pelas abas que você criou!
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<UItemData*> GetItemsByType(EItemType TypeToGet);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetQuantityForItem(UItemData* Item) const;
};
