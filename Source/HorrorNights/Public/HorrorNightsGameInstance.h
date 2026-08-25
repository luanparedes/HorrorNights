// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "HorrorNightsGameInstance.generated.h"

UCLASS()
class HORRORNIGHTS_API UHorrorNightsGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
	virtual void Init() override;
    void ConfigureGraphics();
};