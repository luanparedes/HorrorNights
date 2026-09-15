// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RenderQualityInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class URenderQualityInterface : public UInterface
{
    GENERATED_BODY()
};

class IRenderQualityInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, Category = "Rendering|Capabilities")
    void OnHighEndRenderingEnabled();

    UFUNCTION(BlueprintNativeEvent, Category = "Rendering|Capabilities")
    void OnFallbackRenderingEnabled();
};