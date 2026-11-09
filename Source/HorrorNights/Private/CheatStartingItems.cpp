// Sunnymoon Softwares - All rights reserved

#include "CheatStartingItems.h"
#include "PlayerCharacter.h"
#include "InventoryComponent.h"

UCheatStartingItems::UCheatStartingItems()
{
	bReapplyOnCharacterSwap = false;
}

void UCheatStartingItems::ApplyToPlayer_Implementation(APlayerCharacter* Player)
{
	if (!Player || !Player->Inventory)
		return;

	for (const TPair<TObjectPtr<UItemData>, int32>& Pair : Items)
	{
		Player->Inventory->AddItem(Pair.Key, Pair.Value);
	}
}
