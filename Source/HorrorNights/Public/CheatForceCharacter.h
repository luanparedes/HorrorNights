// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "CheatDefinition.h"
#include "CheatForceCharacter.generated.h"

class UPlayerClassData;

UCLASS(meta = (DisplayName = "Cheat - Force Character"))
class HORRORNIGHTS_API UCheatForceCharacter : public UCheatDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cheat|Character")
	TObjectPtr<UPlayerClassData> ForcedClass;

	UCheatForceCharacter();
	virtual UPlayerClassData* ModifyCharacterClass_Implementation(UPlayerClassData* CurrentClass) override;
};
