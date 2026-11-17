// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "AnimalAnimInstance.generated.h"

class AAnimalCharacter;

UCLASS()
class HORRORNIGHTS_API UAnimalAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Animal|Locomotion")
	float GroundSpeed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Animal|Locomotion")
	float LocomotionBlend = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Animal|Locomotion")
	float WalkSpeed = 150.f;

	UPROPERTY(BlueprintReadOnly, Category = "Animal|Locomotion")
	float RunSpeed = 600.f;

	UPROPERTY(BlueprintReadOnly, Category = "Animal|Locomotion")
	float Direction = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Animal|Locomotion")
	float TurnRate = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Animal|State")
	bool bIsMoving = false;

	UPROPERTY(BlueprintReadOnly, Category = "Animal|State")
	bool bIsRunning = false;

	UPROPERTY(BlueprintReadOnly, Category = "Animal|State")
	bool bIsFalling = false;

	UPROPERTY(BlueprintReadOnly, Category = "Animal|State")
	bool bIsDead = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animal|Tuning", meta = (ClampMin = "0"))
	float MovingThreshold = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animal|Tuning", meta = (ClampMin = "0"))
	float LocomotionInterpSpeed = 6.f;

	UPROPERTY(BlueprintReadOnly, Category = "Animal")
	TObjectPtr<AAnimalCharacter> Animal;

private:
	float LastYaw = 0.f;
	bool bHasLastYaw = false;
};
