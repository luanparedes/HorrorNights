// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "CheatDefinition.h"
#include "CheatForceCharacter.generated.h"

class UPlayerClassData;

/** Cheat: pula o sorteio e faz o jogador nascer com um personagem especifico. */
UCLASS(meta = (DisplayName = "Cheat - Force Character"))
class HORRORNIGHTS_API UCheatForceCharacter : public UCheatDefinition
{
	GENERATED_BODY()

public:
	UCheatForceCharacter();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cheat|Character")
	TObjectPtr<UPlayerClassData> ForcedClass;

	virtual UPlayerClassData* ModifyCharacterClass_Implementation(UPlayerClassData* CurrentClass) override;
};
