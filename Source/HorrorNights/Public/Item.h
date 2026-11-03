// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemData.h"
#include "Item.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class APlayerCharacter;
class UNiagaraComponent;

UCLASS()
class HORRORNIGHTS_API AItem : public AActor
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	UItemData* ItemData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	int32 Quantity = 1;

	AItem();
	void Interact(APlayerCharacter* PlayerCalling);

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* ItemMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* ItemEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* PickupZone;

	UPROPERTY()
	APlayerCharacter* OverlappingPlayer;

private:
	void SnapToGround();
};
