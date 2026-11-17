// Sunnymoon Softwares - All rights reserved


#include "AnimalCharacter.h"
#include "AnimalClassData.h"
#include "AnimalAIController.h"
#include "Item.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AAnimalCharacter::AAnimalCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	AIControllerClass = AAnimalAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	bUseControllerRotationYaw = false;
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->bOrientRotationToMovement = true;
		Movement->RotationRate = FRotator(0.f, 360.f, 0.f);
	}
}

void AAnimalCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!ClassData)
	{
		UE_LOG(LogTemp, Error, TEXT("%s: nenhum AnimalClassData configurado."), *GetName());
	}

	if (CurrentHP < 0)
	{
		CurrentHP = ClassData ? ClassData->HP : 100;
	}

	SetRunning(false);
}

void AAnimalCharacter::SetCurrentHP(int32 NewHP)
{
	CurrentHP = NewHP;

	if (CurrentHP <= 0 && !bDead)
	{
		Die(nullptr);
	}
}

void AAnimalCharacter::SetRunning(bool bRunning)
{
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		const float Walk = ClassData ? ClassData->WalkSpeed : 150.f;
		const float Run = ClassData ? ClassData->RunSpeed : 600.f;
		Movement->MaxWalkSpeed = bRunning ? Run : Walk;
	}
}

float AAnimalCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bDead || !HasAuthority())
	{
		return 0.f;
	}

	const float Received = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (Received <= 0.f)
	{
		return 0.f;
	}

	const int32 Defense = ClassData ? ClassData->Defense : 0;
	const int32 Damage = FMath::Max(1, FMath::RoundToInt(Received) - Defense);

	CurrentHP = FMath::Max(0, CurrentHP - Damage);
	OnDamaged(Damage, DamageCauser);

	AActor* Attacker = EventInstigator ? EventInstigator->GetPawn() : DamageCauser;

	if (CurrentHP <= 0)
	{
		Die(Attacker);
	}
	else if (AAnimalAIController* AnimalController = Cast<AAnimalAIController>(GetController()))
	{
		AnimalController->NotifyDamaged(Attacker);
	}

	return static_cast<float>(Damage);
}

bool AAnimalCharacter::AttackTarget(AActor* Target)
{
	if (bDead || !IsValid(Target) || !HasAuthority())
	{
		return false;
	}

	const double Now = GetWorld()->GetTimeSeconds();
	const float Cooldown = ClassData ? ClassData->AttackCooldown : 1.5f;
	const float Range = ClassData ? ClassData->AttackRange : 200.f;

	if (Now - LastAttackTime < Cooldown)
	{
		return false;
	}

	const float Radius = GetCapsuleComponent() ? GetCapsuleComponent()->GetScaledCapsuleRadius() : 0.f;
	if (FVector::Dist2D(GetActorLocation(), Target->GetActorLocation()) > Range + Radius)
	{
		return false;
	}

	LastAttackTime = Now;

	const FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	if (!ToTarget.IsNearlyZero())
	{
		SetActorRotation(ToTarget.Rotation());
	}

	OnAttack(Target);

	const int32 Power = ClassData ? ClassData->Attack : 10;
	UGameplayStatics::ApplyDamage(Target, static_cast<float>(Power), GetController(), this, nullptr);

	return true;
}

void AAnimalCharacter::Kill(AActor* Killer)
{
	if (!bDead && HasAuthority())
	{
		CurrentHP = 0;
		Die(Killer);
	}
}

void AAnimalCharacter::Die(AActor* Killer)
{
	if (bDead)
	{
		return;
	}

	bDead = true;
	CurrentHP = 0;

	if (AController* OwnController = GetController())
	{
		OwnController->StopMovement();
		OwnController->UnPossess();
		OwnController->Destroy();
	}

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		Capsule->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}

	SpawnDrops();
	OnDeath(Killer);

	SetLifeSpan(ClassData ? ClassData->CorpseLifetime : 10.f);
}

void AAnimalCharacter::SpawnDrops()
{
	if (!ClassData || ClassData->Drops.Num() == 0)
	{
		return;
	}

	const int32 Rolls = FMath::RandRange(FMath::Min(ClassData->MinDropRolls, ClassData->MaxDropRolls), FMath::Max(ClassData->MinDropRolls, ClassData->MaxDropRolls));

	FVector Feet = GetActorLocation();
	if (const UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Feet.Z -= Capsule->GetScaledCapsuleHalfHeight();
	}

	for (int32 Roll = 0; Roll < Rolls; ++Roll)
	{
		const int32 Index = PickWeightedItemEntry(ClassData->Drops);
		if (Index == INDEX_NONE)
		{
			return;
		}

		const FItemSpawnEntry& Entry = ClassData->Drops[Index];

		const FVector2D Offset = FVector2D(FMath::VRand()).GetSafeNormal() * FMath::FRandRange(0.f, ClassData->DropScatterRadius);
		const FTransform SpawnTransform(FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f), Feet + FVector(Offset, 80.f));

		AItem* Item = GetWorld()->SpawnActorDeferred<AItem>(Entry.ItemClass, SpawnTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (!Item)
		{
			continue;
		}

		if (Entry.ItemDataOverride)
		{
			Item->ItemData = Entry.ItemDataOverride;
		}
		Item->Quantity = FMath::RandRange(FMath::Min(Entry.MinQuantity, Entry.MaxQuantity), FMath::Max(Entry.MinQuantity, Entry.MaxQuantity));

		Item->FinishSpawning(SpawnTransform);

		if (IsValid(Item) && ClassData->DropLifetime > 0.f)
		{
			Item->SetLifeSpan(ClassData->DropLifetime);
		}
	}
}
