// Sunnymoon Softwares - All rights reserved

#include "CheatGameplayEffects.h"
#include "PlayerCharacter.h"
#include "GameplayEffect.h"

void UCheatGameplayEffects::ApplyToPlayer_Implementation(APlayerCharacter* Player)
{
	if (!Player)
		return;

	for (const TSubclassOf<UGameplayEffect>& Effect : Effects)
	{
		Player->ApplyItemEffect(Effect);
	}
}
