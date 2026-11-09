// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "CheatDefinition.h"
#include "CheatGameplayEffects.generated.h"

class UGameplayEffect;

/**
 * Cheat: aplica Gameplay Effects (GAS) no jogador ao nascer.
 * Serve para quase tudo que mexe em atributo: modo deus (GE infinito de HP),
 * super velocidade, ataque x10, energia infinita etc. — basta criar o GE.
 */
UCLASS(meta = (DisplayName = "Cheat - Gameplay Effects"))
class HORRORNIGHTS_API UCheatGameplayEffects : public UCheatDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cheat|Effects")
	TArray<TSubclassOf<UGameplayEffect>> Effects;

	virtual void ApplyToPlayer_Implementation(APlayerCharacter* Player) override;
};
