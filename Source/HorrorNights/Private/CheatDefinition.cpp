// Sunnymoon Softwares - All rights reserved

#include "CheatDefinition.h"

FPrimaryAssetId UCheatDefinition::GetPrimaryAssetId() const
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return FPrimaryAssetId();
	}

	return FPrimaryAssetId(GetCheatAssetType(), GetFName());
}

UPlayerClassData* UCheatDefinition::ModifyCharacterClass_Implementation(UPlayerClassData* CurrentClass)
{
	return CurrentClass;
}

void UCheatDefinition::ApplyToPlayer_Implementation(APlayerCharacter* Player)
{
}
