// Sunnymoon Softwares - All rights reserved


#include "ChangeTimeMessageWidget.h"

void UChangeTimeMessageWidget::SetMessageText(const FString& InMessage)
{
    if (DisplayText)
    {
        DisplayText->SetText(FText::FromString(InMessage));
        PlayFadeAnimation();
    }
}