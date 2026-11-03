// Sunnymoon Softwares - All rights reserved


#include "WolfAIController.h"
#include "PlayerCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"

AWolfAIController::AWolfAIController()
{
	// Instanciando o componente e a config
	WolfPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("WolfPerception"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	// Regulando a "miopia" do bicho
	SightConfig->SightRadius = 4000.f; // Distância máxima que ele enxerga
	SightConfig->LoseSightRadius = 5000.f; // Distância pra ele esquecer que viu
	SightConfig->PeripheralVisionAngleDegrees = 90.f; // Visão periférica
	SightConfig->SetMaxAge(5.f); // Tempo que ele memoriza o alvo após sumir

	// Detectar tudo que se mexer
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	// Aplicando as configurações no componente principal
	WolfPerception->ConfigureSense(*SightConfig);
	WolfPerception->SetDominantSense(SightConfig->GetSenseImplementation());

	// Amarrando a nossa função ao evento nativo da Unreal
	WolfPerception->OnTargetPerceptionUpdated.AddDynamic(this, &AWolfAIController::OnTargetDetected);
}

void AWolfAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (BehaviorTree != nullptr)
	{
		RunBehaviorTree(BehaviorTree);

		if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
		{
			FVector LocalDaCabana = FVector(35790.0f, 22930.0, 0.0f);
			BlackboardComp->SetValueAsVector(FName("CabinWay"), LocalDaCabana);

			// ADICIONE ESTAS DUAS LINHAS PARA GARANTIR QUE ELE NASÇA CEGO:
			BlackboardComp->ClearValue(FName("PlayerSighted"));
			BlackboardComp->ClearValue(FName("LastKnownPlayerLocation"));
		}
	}
}

void AWolfAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(Actor);

	if (PlayerCharacter != nullptr && GetBlackboardComponent() != nullptr)
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			// O lobo te viu! A primeira coisa é CANCELAR o timer de esquecer caso ele estivesse rodando
			GetWorld()->GetTimerManager().ClearTimer(LoseSightTimer);

			GetBlackboardComponent()->SetValueAsObject(FName("PlayerSighted"), Actor);
			GetBlackboardComponent()->ClearValue(FName("LastKnownPlayerLocation"));
		}
		else
		{
			// O lobo te perdeu de vista! Salva onde você tava...
			GetBlackboardComponent()->SetValueAsVector(FName("LastKnownPlayerLocation"), Actor->GetActorLocation());

			// MAS AO INVÉS DE LIMPAR A VISÃO NA MESMA HORA, DAMOS 3 SEGUNDOS DE MEMÓRIA:
			GetWorld()->GetTimerManager().SetTimer(LoseSightTimer, this, &AWolfAIController::ForgetPlayer, 3.0f, false);
		}
	}
}

void AWolfAIController::ForgetPlayer()
{
	// Essa função só roda se passarem 3 segundos inteiros sem o lobo te ver
	if (GetBlackboardComponent())
	{
		GetBlackboardComponent()->ClearValue(FName("PlayerSighted"));
	}
}