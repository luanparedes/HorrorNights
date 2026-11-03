// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "WolfAIController.generated.h"

UCLASS()
class HORRORNIGHTS_API AWolfAIController : public AAIController
{
	GENERATED_BODY()

public:
	AWolfAIController();

protected:
	// Roda assim que o controlador "possui" o lobo no mapa
	virtual void OnPossess(APawn* InPawn) override;

private:
	FTimerHandle LoseSightTimer;

	// O componente de percepção (os olhos)
	UPROPERTY(VisibleAnywhere, Category = "IA")
	class UAIPerceptionComponent* WolfPerception;

	// As regras da visão (distância, ângulo)
	UPROPERTY(VisibleAnywhere, Category = "IA")
	class UAISenseConfig_Sight* SightConfig;

	// Espaço pra você selecionar a Behavior Tree do lobo lá na Blueprint depois
	UPROPERTY(EditDefaultsOnly, Category = "IA")
	class UBehaviorTree* BehaviorTree;

	// Função engatilhada quando ele vê ou perde alguém de vista
	UFUNCTION()
	void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
	void ForgetPlayer();
};