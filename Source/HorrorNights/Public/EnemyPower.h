// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "EnemyPower.generated.h"

UENUM(BlueprintType)
enum class EEnemyPower : uint8
{
	ExtremelyWeak   UMETA(DisplayName = "Extremely Weak"),
	VeryWeak	    UMETA(DisplayName = "Very Weak"),
	Weak            UMETA(DisplayName = "Weak"),
	Normal          UMETA(DisplayName = "Normal"),
	Strong          UMETA(DisplayName = "Strong"),
	VeryStrong      UMETA(DisplayName = "Very Strong"),
	ExtremelyStrong UMETA(DisplayName = "Extremely Strong")
};