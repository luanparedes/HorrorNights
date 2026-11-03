// Sunnymoon Softwares - All rights reserved


#include "EnemyStatsComponent.h"
#include "EnemyCharacter.h"

UEnemyStatsComponent::UEnemyStatsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEnemyStatsComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UEnemyStatsComponent::InitializeStats()
{
	if (CurrentClassData != nullptr)
	{
		HP = (CurrentClassData->HP) * GetMultiplierFromPower();
		AttackPower = (CurrentClassData->Attack) * GetMultiplierFromPower();
		DefensePower = (CurrentClassData->Defense) * GetMultiplierFromPower();
		MagicAttack = (CurrentClassData->MagicAttack) * GetMultiplierFromPower();
		Speed = (CurrentClassData->Speed) * GetMultiplierFromPower();

		AEnemyCharacter* EnemyCharacter = Cast<AEnemyCharacter>(GetOwner());

		if (EnemyCharacter)
		{
			UCharacterMovementComponent* MoveComp = EnemyCharacter->GetCharacterMovement();

			if (MoveComp)
			{
				MoveComp->MaxWalkSpeed = static_cast<float>(Speed);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ERROR: No Data Asset assigned to the component."));
	}
}

float UEnemyStatsComponent::GetMultiplierFromPower()
{
	switch (EnemyPower)
	{
		case EEnemyPower::ExtremelyWeak:
			return 0.25f;
		case EEnemyPower::VeryWeak:
			return 0.5f;
		case EEnemyPower::Weak:
			return 0.75f;
		case EEnemyPower::Normal:
			return 1.0f;
		case EEnemyPower::Strong:
			return 1.5;
		case EEnemyPower::VeryStrong:
			return 2.0;
		case EEnemyPower::ExtremelyStrong:
			return 3.0;
		default:
			return 1.0f;
	}
}


