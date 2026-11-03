#include "BTTask_ClearBlackboard.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_ClearBlackboard::UBTTask_ClearBlackboard()
{
	// É esse texto aqui que vai aparecer na lista roxa da Behavior Tree
	NodeName = TEXT("Clear Blackboard Value");
}

EBTNodeResult::Type UBTTask_ClearBlackboard::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Puxa o caderninho da IA
	if (UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent())
	{
		// Limpa a chave exata que você selecionar no painel Details do editor
		BBComp->ClearValue(GetSelectedBlackboardKey());

		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}