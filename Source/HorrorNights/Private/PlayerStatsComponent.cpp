// Sunnymoon Softwares - All rights reserved


#include "PlayerStatsComponent.h"
#include "PlayerCharacter.h"

UPlayerStatsComponent::UPlayerStatsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerStatsComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPlayerStatsComponent::InitializeStats()
{
	if (CurrentClassData != nullptr)
	{
		HP = CurrentClassData->HP;
		Energy = CurrentClassData->Energy;
		AttackPower = CurrentClassData->Attack;
		DefensePower = CurrentClassData->Defense;
		Smartness = CurrentClassData->Smartness;
		MagicAttack = CurrentClassData->MagicAttack;
		Speed = CurrentClassData->Speed;
		JumpStat = CurrentClassData->Jump;

		APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetOwner());

		if (PlayerCharacter)
		{
			UCharacterMovementComponent* MoveComp = PlayerCharacter->GetCharacterMovement();

			if (MoveComp)
			{
				MoveComp->MaxWalkSpeed = static_cast<float>(CurrentClassData->Speed);
				MoveComp->JumpZVelocity = static_cast<float>(CurrentClassData->Jump);

				UE_LOG(LogTemp, Warning, TEXT("Status aplicados com sucesso! Speed: %f | Jump: %f"), MoveComp->MaxWalkSpeed, MoveComp->JumpZVelocity);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ERROR: No Data Asset assigned to the component."));
	}
}
