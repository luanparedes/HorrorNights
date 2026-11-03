// Sunnymoon Softwares - All rights reserved


#include "InventoryComponent.h"
#include "PlayerCharacter.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::UseItem(UItemData* Item)
{
	if (!Item || !InventoryItems.Contains(Item))
		return;

	if (APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwner()))
	{
		if (Item->ItemEffect)
		{
			Player->ApplyItemEffect(Item->ItemEffect);
			RemoveItem(Item);
		}
	}
}

void UInventoryComponent::AddItem(UItemData* Item, int32 Quantity)
{
	if (!Item || Quantity <= 0) 
		return;

	if (InventoryItems.Contains(Item))
	{
		InventoryItems[Item] += Quantity;
	}
	else
	{
		InventoryItems.Add(Item, Quantity);
	}
}

void UInventoryComponent::RemoveItem(UItemData* Item, int32 Quantity)
{
	if (!Item || Quantity <= 0 || !InventoryItems.Contains(Item)) 
		return;

	InventoryItems[Item] -= Quantity;

	if (InventoryItems[Item] <= 0)
	{
		InventoryItems.Remove(Item);
	}
}

TArray<UItemData*> UInventoryComponent::GetItemsByType(EItemType TypeToGet)
{
	TArray<UItemData*> FilteredItems;

	for (const TTuple<UItemData*, int32>& Pair : InventoryItems)
	{
		if (Pair.Key && Pair.Key->ItemType == TypeToGet)
		{
			FilteredItems.Add(Pair.Key);
		}
	}

	return FilteredItems;
}

int32 UInventoryComponent::GetQuantityForItem(UItemData* Item) const
{
	if (Item && InventoryItems.Contains(Item))
	{
		return InventoryItems[Item];
	}

	return 0;
}