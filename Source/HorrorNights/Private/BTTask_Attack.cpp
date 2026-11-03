// Sunnymoon Softwares - All rights reserved

#include "BTTask_Attack.h"
#include "AIController.h"
#include "GameFramework/Character.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = TEXT("Atacar Jogador");
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		if (ACharacter* Lobo = Cast<ACharacter>(AIController->GetPawn()))
		{
			// É AQUI QUE A MÁGICA ACONTECE!
			// Você pode mandar ele tocar uma Animation Montage de mordida:
			// Lobo->PlayAnimMontage(SuaMontageDeAtaque);

			// Ou chamar uma função específica que você já tenha criado no C++ do lobo:
			// Lobo->RealizarAtaqueMelee();

			// Imprime na tela só pra gente ter certeza que ele tentou morder antes de colocar a animação
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("NHAC! Wolf attacked!"));

			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}

