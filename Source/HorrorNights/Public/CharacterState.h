// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PlayerClassData.h"
#include "CharacterState.generated.h"

UCLASS()
class HORRORNIGHTS_API ACharacterState : public APlayerState
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Class")
	UPlayerClassData* SortedCharacter;
};
