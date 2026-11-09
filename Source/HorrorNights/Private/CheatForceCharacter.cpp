// Sunnymoon Softwares - All rights reserved

#include "CheatForceCharacter.h"
#include "PlayerClassData.h"

UCheatForceCharacter::UCheatForceCharacter()
{
	ExclusiveGroup = TEXT("Character");

	bCanApplyDuringGameplay = false;
	bRespawnWhenActivatedInGame = true;
}

UPlayerClassData* UCheatForceCharacter::ModifyCharacterClass_Implementation(UPlayerClassData* CurrentClass)
{
	return ForcedClass ? ForcedClass.Get() : CurrentClass;
}
