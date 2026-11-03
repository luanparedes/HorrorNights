// Sunnymoon Softwares - All rights reserved


#include "EnemyCharacter.h"

AEnemyCharacter::AEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	EnemyStats = CreateDefaultSubobject<UEnemyStatsComponent>(TEXT("Enemy Stats"));
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (EnemyStats)
	{
		EnemyStats->InitializeStats();
	}
}

void AEnemyCharacter::SetMovementSpeed(bool bIsRunning)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = bIsRunning ? EnemyStats->Speed : (EnemyStats->Speed * 0.3f);
	}
}

void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEnemyCharacter::Attack()
{
	// Implement enemy attack logic here
}

void AEnemyCharacter::Defense()
{
	// Implement enemy defense logic here
}

void AEnemyCharacter::Magic()
{
	// Implement enemy magic logic here
}

void AEnemyCharacter::Ability()
{
	// Implement enemy ability logic here
}