// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerStatsComponent.h"
#include "BaseCharacter.generated.h"

UCLASS(Abstract)
class HORRORNIGHTS_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void Attack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void Defense();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void Magic();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void Ability();

	ABaseCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

private:
};
