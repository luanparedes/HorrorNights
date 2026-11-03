// Sunnymoon Softwares - All rights reserved

#include "PlayerAttributeSet.h"
#include "GameplayEffectExtension.h"

UPlayerAttributeSet::UPlayerAttributeSet()
{

}

void UPlayerAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		float LocalDamageDone = GetIncomingDamage();

		SetIncomingDamage(0.0f);

		if (LocalDamageDone > 0.0f)
		{
			float NewHP = GetHP() - LocalDamageDone;
			SetHP(FMath::Clamp(NewHP, 0.0f, GetMaxHP()));
			OnHealthChanged.Broadcast(GetHP());
		}
	}

	if (Data.EvaluatedData.Attribute == GetHPAttribute())
	{
		SetHP(FMath::Clamp(GetHP(), 0.0f, GetMaxHP()));
		OnHealthChanged.Broadcast(GetHP());
	}

	if (Data.EvaluatedData.Attribute == GetMaxHPAttribute())
	{
		OnHealthChanged.Broadcast(GetMaxHP());
	}

	if (Data.EvaluatedData.Attribute == GetEnergyAttribute())
	{
		SetEnergy(FMath::Clamp(GetEnergy(), 0.0f, GetMaxEnergy()));
		OnEnergyChanged.Broadcast(GetEnergy());
	}
}