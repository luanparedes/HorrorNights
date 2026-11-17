// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "AnimalCharacter.generated.h"

class UAnimalClassData;

UCLASS()
class HORRORNIGHTS_API AAnimalCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AAnimalCharacter();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animal")
	TObjectPtr<UAnimalClassData> ClassData;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category = "Animal")
	bool AttackTarget(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "Animal")
	void SetRunning(bool bRunning);

	UFUNCTION(BlueprintCallable, Category = "Animal")
	void Kill(AActor* Killer = nullptr);

	UFUNCTION(BlueprintPure, Category = "Animal")
	int32 GetCurrentHP() const { return CurrentHP; }

	UFUNCTION(BlueprintPure, Category = "Animal")
	bool IsDead() const { return bDead; }

	void SetCurrentHP(int32 NewHP);

	UFUNCTION(BlueprintImplementableEvent, Category = "Animal")
	void OnAttack(AActor* Target);

	UFUNCTION(BlueprintImplementableEvent, Category = "Animal")
	void OnDamaged(int32 Damage, AActor* DamageCauser);

	UFUNCTION(BlueprintImplementableEvent, Category = "Animal")
	void OnStartFleeing();

	UFUNCTION(BlueprintImplementableEvent, Category = "Animal")
	void OnDeath(AActor* Killer);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Animal")
	int32 CurrentHP = -1;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Animal")
	bool bDead = false;

private:
	double LastAttackTime = -1000.0;

	void Die(AActor* Killer);
	void SpawnDrops();
};
