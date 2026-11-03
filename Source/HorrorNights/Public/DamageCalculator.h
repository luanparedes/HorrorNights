// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "DamageCalculator.generated.h"

UCLASS()
class HORRORNIGHTS_API UDamageCalculator : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
	
public:
	UDamageCalculator();
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
