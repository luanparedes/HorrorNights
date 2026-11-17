// Sunnymoon Softwares - All rights reserved


#include "AnimalAIController.h"
#include "AnimalCharacter.h"
#include "AnimalClassData.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Components/CapsuleComponent.h"

AAnimalAIController::AAnimalAIController()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAnimalAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	Animal = Cast<AAnimalCharacter>(InPawn);
	if (!Animal)
	{
		return;
	}

	EnterIdle();
	IdleUntil = Now() + FMath::FRandRange(0.f, 2.f);

	GetWorldTimerManager().SetTimer(ThinkTimer, this, &AAnimalAIController::Think, ThinkInterval, true, FMath::FRandRange(0.f, ThinkInterval));
}

void AAnimalAIController::OnUnPossess()
{
	GetWorldTimerManager().ClearTimer(ThinkTimer);
	Animal = nullptr;

	Super::OnUnPossess();
}

const UAnimalClassData* AAnimalAIController::GetData() const
{
	if (Animal && Animal->ClassData)
	{
		return Animal->ClassData;
	}
	return GetDefault<UAnimalClassData>();
}

double AAnimalAIController::Now() const
{
	return GetWorld()->GetTimeSeconds();
}

bool AAnimalAIController::IsMoving() const
{
	return GetMoveStatus() == EPathFollowingStatus::Moving;
}

void AAnimalAIController::SetState(EAnimalState NewState)
{
	State = NewState;
	StateStartTime = Now();
}

void AAnimalAIController::Think()
{
	if (!Animal || Animal->IsDead())
	{
		return;
	}

	const UAnimalClassData* Data = GetData();
	AActor* NearbyPlayer = FindNearestPlayer(Data->DetectionRadius);

	switch (State)
	{
	case EAnimalState::Attack:
	{
		AActor* Target = Threat.Get();
		if (!IsValid(Target) || FVector::Dist2D(Target->GetActorLocation(), Animal->GetActorLocation()) > Data->DetectionRadius * 2.f)
		{
			EnterIdle();
			break;
		}

		if (Now() - StateStartTime > Data->MaxAttackTime)
		{
			StartFlee(Target);
			break;
		}

		const float Radius = Animal->GetCapsuleComponent() ? Animal->GetCapsuleComponent()->GetScaledCapsuleRadius() : 0.f;
		const float Distance = FVector::Dist2D(Target->GetActorLocation(), Animal->GetActorLocation());

		if (Distance > Data->AttackRange + Radius)
		{
			MoveToActor(Target, Data->AttackRange * 0.5f);
		}
		else
		{
			StopMovement();
			Animal->AttackTarget(Target);
		}
		break;
	}

	case EAnimalState::Flee:
	{
		AActor* From = Threat.Get();
		const bool bThreatened = IsValid(From) && FVector::Dist2D(From->GetActorLocation(), Animal->GetActorLocation()) < Data->DetectionRadius * 1.5f;

		if (bThreatened)
		{
			if (!IsMoving() && !MoveAwayFrom(From))
			{
				StartAttack(From);
			}
		}
		else if (Now() - StateStartTime >= Data->MinFleeTime || !IsMoving())
		{
			EnterIdle();
		}
		break;
	}

	case EAnimalState::Idle:
	case EAnimalState::Wander:
	{
		if (!NearbyPlayer)
		{
			bReactedToNearbyPlayer = false;
		}
		else if (!bReactedToNearbyPlayer)
		{
			bReactedToNearbyPlayer = true;

			if (FMath::FRand() < Data->AttackChanceWhenApproached)
			{
				StartAttack(NearbyPlayer);
				break;
			}

			if (FMath::FRand() < Data->FleeChanceWhenApproached)
			{
				StartFlee(NearbyPlayer);
				break;
			}
		}

		if (State == EAnimalState::Idle && Now() >= IdleUntil)
		{
			StartWander();
		}
		else if (State == EAnimalState::Wander && !IsMoving())
		{
			EnterIdle();
		}
		break;
	}
	}
}

void AAnimalAIController::NotifyDamaged(AActor* Attacker)
{
	if (!Animal || Animal->IsDead())
	{
		return;
	}

	bReactedToNearbyPlayer = true;

	if (!IsValid(Attacker))
	{
		if (State != EAnimalState::Flee)
		{
			StartFlee(nullptr);
		}
		return;
	}

	if (State == EAnimalState::Attack)
	{
		Threat = Attacker;
		return;
	}

	if (FMath::FRand() < GetData()->RetaliateChanceWhenHit)
	{
		StartAttack(Attacker);
	}
	else
	{
		StartFlee(Attacker);
	}
}

AActor* AAnimalAIController::FindNearestPlayer(float Radius) const
{
	AActor* Nearest = nullptr;
	float NearestDistSquared = FMath::Square(Radius);
	const FVector Location = Animal->GetActorLocation();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PlayerController = It->Get();
		APawn* PlayerPawn = PlayerController ? PlayerController->GetPawn() : nullptr;

		if (!PlayerPawn)
		{
			continue;
		}

		const float DistSquared = FVector::DistSquared(PlayerPawn->GetActorLocation(), Location);
		if (DistSquared < NearestDistSquared)
		{
			NearestDistSquared = DistSquared;
			Nearest = PlayerPawn;
		}
	}
	return Nearest;
}

void AAnimalAIController::EnterIdle()
{
	const UAnimalClassData* Data = GetData();

	SetState(EAnimalState::Idle);
	StopMovement();
	Threat.Reset();
	IdleUntil = Now() + FMath::FRandRange(FMath::Min(Data->IdleTimeMin, Data->IdleTimeMax), FMath::Max(Data->IdleTimeMin, Data->IdleTimeMax));

	if (Animal)
	{
		Animal->SetRunning(false);
	}
}

void AAnimalAIController::StartWander()
{
	const UAnimalClassData* Data = GetData();
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());

	if (!Animal || !NavSys)
	{
		EnterIdle();
		return;
	}

	const FVector Location = Animal->GetActorLocation();
	const float Distance = FMath::FRandRange(FMath::Min(Data->WanderMinDistance, Data->WanderMaxDistance), FMath::Max(Data->WanderMinDistance, Data->WanderMaxDistance));
	const FVector Direction = FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f).Vector();

	FNavLocation Destination;
	bool bFound = NavSys->ProjectPointToNavigation(Location + Direction * Distance, Destination, FVector(500.f, 500.f, 2000.f));

	if (!bFound)
	{
		bFound = NavSys->GetRandomReachablePointInRadius(Location, Distance, Destination);
	}

	if (!bFound)
	{
		EnterIdle();
		return;
	}

	SetState(EAnimalState::Wander);
	Animal->SetRunning(false);

	if (MoveToLocation(Destination.Location, 50.f) == EPathFollowingRequestResult::Failed)
	{
		EnterIdle();
	}
}

void AAnimalAIController::StartFlee(AActor* From)
{
	if (!Animal)
	{
		return;
	}

	SetState(EAnimalState::Flee);
	Threat = From;
	Animal->SetRunning(true);
	Animal->OnStartFleeing();

	if (!MoveAwayFrom(From) && IsValid(From))
	{
		StartAttack(From);
	}
}

void AAnimalAIController::StartAttack(AActor* Target)
{
	if (!Animal || !IsValid(Target))
	{
		EnterIdle();
		return;
	}

	SetState(EAnimalState::Attack);
	Threat = Target;
	Animal->SetRunning(true);
}

bool AAnimalAIController::MoveAwayFrom(const AActor* From)
{
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!Animal || !NavSys)
	{
		return false;
	}

	const FVector Location = Animal->GetActorLocation();
	FVector Away = IsValid(From) ? (Location - From->GetActorLocation()).GetSafeNormal2D() : FVector::ZeroVector;

	if (Away.IsNearlyZero())
	{
		Away = FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f).Vector();
	}

	static const float Angles[] = { 0.f, 35.f, -35.f, 70.f, -70.f, 100.f, -100.f };
	const float FleeDistance = GetData()->FleeDistance;

	for (const float Angle : Angles)
	{
		const FVector Direction = Away.RotateAngleAxis(Angle + FMath::FRandRange(-10.f, 10.f), FVector::UpVector);

		FNavLocation Destination;
		if (!NavSys->ProjectPointToNavigation(Location + Direction * FleeDistance, Destination, FVector(600.f, 600.f, 2000.f)))
		{
			continue;
		}

		if (MoveToLocation(Destination.Location, 100.f) != EPathFollowingRequestResult::Failed)
		{
			return true;
		}
	}

	return false;
}
