// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CheatDefinition.generated.h"

class APlayerCharacter;
class UPlayerClassData;

/**
 * Base de todos os cheats do jogo.
 *
 * Cada cheat eh um Data Asset: voce cria um asset no Content Browser escolhendo
 * uma das subclasses (ForceCharacter, StartingItems, GameplayEffects...) e preenche
 * o codigo + os parametros. Tambem da pra criar cheats novos em Blueprint
 * herdando desta classe e sobrescrevendo os eventos abaixo.
 *
 * O cheat "age" em dois momentos do spawn do jogador:
 *  1) ModifyCharacterClass -> antes do pawn ser escolhido (GameMode)
 *  2) ApplyToPlayer        -> depois que o pawn foi possuido e rodou o BeginPlay
 *
 * Descoberta automatica: todo cheat se registra no Asset Manager com o tipo "Cheat".
 * Basta salvar o asset dentro de /Game/DataAssets/Cheats (ou subpastas) que o
 * CheatSubsystem encontra sozinho — nao existe lista para manter.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class HORRORNIGHTS_API UCheatDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Tipo usado no Asset Manager (Project Settings > Asset Manager > Primary Asset Types to Scan). */
	static FPrimaryAssetType GetCheatAssetType() { return FPrimaryAssetType(TEXT("Cheat")); }

	/** Todas as subclasses compartilham o mesmo tipo "Cheat", pra serem achadas juntas. */
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cheat")
	FString Code;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cheat")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cheat", meta = (MultiLine = true))
	FText Description;

	/**
	 * Cheats do mesmo grupo se excluem: ativar um desativa os outros.
	 * Ex: todos os "jogar com personagem X" usam o grupo "Character".
	 * Vazio = nao conflita com ninguem.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cheat")
	FName ExclusiveGroup;

	/**
	 * Se ativado com a partida rolando, aplica na hora no personagem atual.
	 * Desmarque para cheats que so fazem sentido no spawn (ex: trocar de personagem):
	 * eles ficam ativos e passam a valer no proximo spawn.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cheat")
	bool bCanApplyDuringGameplay = true;

	/**
	 * Se ativado com a partida rolando, faz respawn do jogador na mesma posicao
	 * (ex: trocar de personagem). O inventario eh transferido pro personagem novo.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cheat")
	bool bRespawnWhenActivatedInGame = false;

	/**
	 * Reaplicar este cheat no personagem novo quando houver troca de personagem?
	 * Marque para efeitos no corpo (GAS). Desmarque para o que ja foi transferido (itens).
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cheat")
	bool bReapplyOnCharacterSwap = true;

	/** Chamado pelo GameMode antes de escolher o pawn. Retorne a classe que deve ser usada. */
	UFUNCTION(BlueprintNativeEvent, Category = "Cheat")
	UPlayerClassData* ModifyCharacterClass(UPlayerClassData* CurrentClass);

	/** Chamado no servidor logo apos o jogador possuir o personagem (GAS ja inicializado). */
	UFUNCTION(BlueprintNativeEvent, Category = "Cheat")
	void ApplyToPlayer(APlayerCharacter* Player);
};
