// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Containers/UnrealString.h"
#include "ChangeTimeMessageWidget.generated.h"

UCLASS()
class HORRORNIGHTS_API UChangeTimeMessageWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    // Chama a animação que você vai criar no editor
    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void PlayFadeAnimation();

    // Método para atualizar o texto da mensagem
    void SetMessageText(const FString& InMessage);

protected:
    // O nome aqui TEM que ser exatamente o mesmo do elemento no Widget Blueprint
    UPROPERTY(meta = (BindWidget))
    UTextBlock* DisplayText;
};
