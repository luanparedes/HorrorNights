// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "InputActionValue.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h" 
#include "InventoryComponent.h"
#include "PlayerCharacter.generated.h"

class UCapsuleComponent;
class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class AItem;
class UPlayerClassData;
class UAbilitySystemComponent;
class UPlayerAttributeSet;
class UInventoryComponent;

UCLASS()
class HORRORNIGHTS_API APlayerCharacter : public ABaseCharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Stats")
    UPlayerClassData* ClassData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
    UPlayerAttributeSet* PlayerAttributes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    UInventoryComponent* Inventory;

    UPROPERTY(VisibleAnywhere)
    AItem* ItemOnRange;

    APlayerCharacter();
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    virtual void Tick(float DeltaTime) override;
    virtual void PossessedBy(AController* NewController) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void Attack() override;
    virtual void Defense() override;
    virtual void Magic() override;
    virtual void Ability() override;

    virtual void OnSpeedChanged(const FOnAttributeChangeData& Data);
    virtual void OnJumpChanged(const FOnAttributeChangeData& Data);
    virtual void OnHPChanged(const FOnAttributeChangeData& Data);
    virtual void OnEnergyChanged(const FOnAttributeChangeData& Data);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ApplyItemEffect(TSubclassOf<class UGameplayEffect> EffectToApply);

    UFUNCTION(Server, Reliable, BlueprintCallable)
    void ServerRPCFunction();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities", meta = (AllowPrivateAccess = "true"))
    UAbilitySystemComponent* AbilitySystemComponent;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UUserWidget> InventoryWidgetClass;

    UPROPERTY()
    UUserWidget* InventoryWidgetInstance;

    bool bIsInventoryOpen = false;
    bool bPlayerReady = false;
    bool bCheatsApplied = false;
	bool bIsPreparingAttack = false;

    UFUNCTION(BlueprintImplementableEvent, Category = "Player")
    void OnPlayerReady();

    void TryFinishSetup();
    void TryApplyCheats();
    virtual void NotifyControllerChanged() override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
	void OpenInventory(const FInputActionValue& Value);
    void Interact(const FInputActionValue& Value);
    void StartPrepareAttack(const FInputActionValue& Value);
    void TriggerAttack(const FInputActionValue& Value);
	void StopPrepareAttack(const FInputActionValue& Value);

private:
    UPROPERTY(EditAnywhere)
    USpringArmComponent* CameraBoom;

    UPROPERTY(EditAnywhere)
    UCameraComponent* CameraComp;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputMappingContext* InputMapping;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* LookAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* OpenInventoryAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* InteractAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* AttackPrepareAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* AttackTriggerAction;
};