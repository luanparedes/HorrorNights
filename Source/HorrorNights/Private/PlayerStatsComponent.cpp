// Sunnymoon Softwares - All rights reserved


#include "PlayerStatsComponent.h"

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
		CurrentHP = CurrentClassData->HP;
		CurrentEnergy = CurrentClassData->Energy;
		CurrentInventory = CurrentClassData->Inventory;
		CurrentHabilities = CurrentClassData->Habilities;
		FString test = CurrentClassData->CharacterBlueprint->GetName();

		UE_LOG(LogTemp, Warning, TEXT("Class %s Initial Status! Max HP: %d and Max Energy: %d"), *test, CurrentHP, CurrentEnergy);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ERROR: No Data Asset assigned to the component."));
	}
}
