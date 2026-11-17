// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AnimalAIController.generated.h"

class AAnimalCharacter;
class UAnimalClassData;

UENUM(BlueprintType)
enum class EAnimalState : uint8
{
	Idle,
	Wander,
	Flee,
	Attack
};

UCLASS()
class HORRORNIGHTS_API AAnimalAIController : public AAIController
{
	GENERATED_BODY()

public:
	AAnimalAIController();

	void NotifyDamaged(AActor* Attacker);

	UFUNCTION(BlueprintPure, Category = "Animal AI")
	EAnimalState GetAnimalState() const { return State; }

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UPROPERTY(EditDefaultsOnly, Category = "Animal AI", meta = (ClampMin = "0.05"))
	float ThinkInterval = 0.25f;

private:
	UPROPERTY()
	TObjectPtr<AAnimalCharacter> Animal;

	TWeakObjectPtr<AActor> Threat;
	EAnimalState State = EAnimalState::Idle;
	double StateStartTime = 0.0;
	double IdleUntil = 0.0;

	bool bReactedToNearbyPlayer = false;

	FTimerHandle ThinkTimer;

	const UAnimalClassData* GetData() const;
	void Think();
	AActor* FindNearestPlayer(float Radius) const;

	void EnterIdle();
	void StartWander();
	void StartFlee(AActor* From);
	void StartAttack(AActor* Target);
	bool MoveAwayFrom(const AActor* From);
	void SetState(EAnimalState NewState);
	bool IsMoving() const;
	double Now() const;
};
