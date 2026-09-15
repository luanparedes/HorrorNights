// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MainCharacterAnimInstance.generated.h"

class APlayerCharacter;
class UCharacterMovementComponent;

UCLASS()
class HORRORNIGHTS_API UMainCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	APlayerCharacter* Character;

	UPROPERTY(BlueprintReadOnly)
	UCharacterMovementComponent* MovementComponent;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	float GroundSpeed = 0;

	UPROPERTY(BlueprintReadOnly)
	bool IsFalling = false;

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};
