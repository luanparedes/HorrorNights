// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "PlayerAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
        GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
        GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
        GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
        GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChangedDelegate, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxHealthChangedDelegate, float, NewMaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnergyChangedDelegate, float, NewEnergy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMagicCooldownChangedDelegate, float, NewCooldown);

UCLASS()
class HORRORNIGHTS_API UPlayerAttributeSet : public UAttributeSet
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, Category = "Attributes")
    FGameplayAttributeData HP;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, HP)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxHP;
	ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, MaxHP)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Energy;
	ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, Energy)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxEnergy;
	ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, MaxEnergy)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Attack;
	ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, Attack)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Defense;
	ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, Defense)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Smartness;
	ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, Smartness)
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MagicAttack;
	ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, MagicAttack)

    UPROPERTY(BlueprintReadOnly, Category = "Attributes")
    FGameplayAttributeData Speed;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, Speed)

    UPROPERTY(BlueprintReadOnly, Category = "Attributes")
    FGameplayAttributeData Jump;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, Jump)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData WaitMagicTimeMultiplier;
	ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, WaitMagicTimeMultiplier)
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData HealTimeMultiplier;
	ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, HealTimeMultiplier)
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData CookingTimeMultiplier;
	ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, CookingTimeMultiplier)
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData DriveVelocityMultiplier;
	ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, DriveVelocityMultiplier)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, IncomingDamage)

	UPROPERTY(BlueprintAssignable, Category = "UI|Attributes")
	FOnHealthChangedDelegate OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "UI|Attributes")
	FOnMaxHealthChangedDelegate OnMaxHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "UI|Attributes")
	FOnEnergyChangedDelegate OnEnergyChanged;

	UPROPERTY(BlueprintAssignable, Category = "UI|Attributes")
	FOnMagicCooldownChangedDelegate OnMagicCooldownChanged;

	UPlayerAttributeSet();
	void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
};