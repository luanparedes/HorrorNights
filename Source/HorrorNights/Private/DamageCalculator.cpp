// Sunnymoon Softwares - All rights reserved


#include "DamageCalculator.h"
#include "PlayerAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectExecutionCalculation.h"

struct FHorrorDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Attack);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Defense);

	FHorrorDamageStatics()
	{
		// O Ataque vem de quem bate (Source). O "false" significa não pegar snapshot no momento de criação do efeito, e sim na execução.
		DEFINE_ATTRIBUTE_CAPTUREDEF(UPlayerAttributeSet, Attack, Source, false);

		// A Defesa vem de quem apanha (Target).
		DEFINE_ATTRIBUTE_CAPTUREDEF(UPlayerAttributeSet, Defense, Target, false);
	}
};

static const FHorrorDamageStatics& DamageStatics()
{
	static FHorrorDamageStatics DStatics;
	return DStatics;
}

UDamageCalculator::UDamageCalculator()
{
	// Aqui a gente avisa o GAS: "Prepara para ler o Ataque do inimigo e a nossa Defesa!"
	RelevantAttributesToCapture.Add(DamageStatics().AttackDef);
	RelevantAttributesToCapture.Add(DamageStatics().DefenseDef);
}

void UDamageCalculator::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
	UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();

	if (!TargetASC || !SourceASC) 
		return;

	// Prepara os parâmetros de avaliação (lida com as tags que afetam o dano no momento)
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = ExecutionParams.GetOwningSpec().CapturedSourceTags.GetAggregatedTags();
	EvaluationParameters.TargetTags = ExecutionParams.GetOwningSpec().CapturedTargetTags.GetAggregatedTags();

	// Extrai o Ataque do monstro
	float AttackPower = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().AttackDef, EvaluationParameters, AttackPower);

	// Extrai a Defesa do personagem
	float TargetDefense = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DefenseDef, EvaluationParameters, TargetDefense);

	// --- A FÓRMULA DE JOGOS MODERNOS ---
	float MitigationConstant = 100.0f;

	// Trava a defesa no zero caso algum debuff zere demais a armadura, para não quebrar a divisão
	float EffectiveDefense = FMath::Max<float>(TargetDefense, 0.0f);

	// Fator de multiplicação (ex: resulta em 0.5 para 50% de redução)
	float DamageMultiplier = MitigationConstant / (MitigationConstant + EffectiveDefense);

	float FinalDamage = AttackPower * DamageMultiplier;

	// Garante que o mínimo de dano seja sempre 1 (todo ataque causa pelo menos um arranhão)
	FinalDamage = FMath::Max<float>(FinalDamage, 1.0f);

	// Despeja o resultado no Atributo Fantasma que criamos
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
		UPlayerAttributeSet::GetIncomingDamageAttribute(),
		EGameplayModOp::Additive,
		FinalDamage
	));
}