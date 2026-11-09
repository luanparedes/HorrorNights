// Sunnymoon Softwares - All rights reserved

#include "CheatForceCharacter.h"
#include "PlayerClassData.h"

UCheatForceCharacter::UCheatForceCharacter()
{
	// Dois "jogar com X" ao mesmo tempo nao fazem sentido: o ultimo ativado vence.
	ExclusiveGroup = TEXT("Character");

	// Nao da pra "aplicar" troca de personagem num corpo existente: no meio da partida
	// o PlayerController faz respawn na mesma posicao com o personagem novo.
	bCanApplyDuringGameplay = false;
	bRespawnWhenActivatedInGame = true;
}

UPlayerClassData* UCheatForceCharacter::ModifyCharacterClass_Implementation(UPlayerClassData* CurrentClass)
{
	return ForcedClass ? ForcedClass.Get() : CurrentClass;
}
