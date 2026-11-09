// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HorrorNightsPlayerController.generated.h"

class UUserWidget;
class UInputAction;
class UInputMappingContext;
class UCheatSubsystem;

/** Resultado de uma tentativa de ativar cheat, pra UI mostrar a mensagem certa. */
UENUM(BlueprintType)
enum class ECheatActivationResult : uint8
{
	InvalidCode			UMETA(DisplayName = "Invalid Code"),
	AlreadyActive		UMETA(DisplayName = "Already Active"),
	AppliedNow			UMETA(DisplayName = "Applied Now"),
	AppliesOnNextSpawn	UMETA(DisplayName = "Applies On Next Spawn"),
	SentToServer		UMETA(DisplayName = "Sent To Server"),
	CharacterSwapped	UMETA(DisplayName = "Character Swapped")
};

/**
 * PlayerController do jogo, usado no main menu E na partida.
 *
 * Responsabilidades dos cheats:
 *  - Escutar o atalho e abrir/fechar o WBP de cheats.
 *  - Ativar o cheat digitado e, se a partida estiver rolando, aplicar na hora.
 *  - Em multiplayer, mandar o pedido pro servidor (quem manda no jogo).
 *
 * O estado (quais cheats estao ativos) continua no CheatSubsystem, no GameInstance.
 */
UCLASS()
class HORRORNIGHTS_API AHorrorNightsPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/** Abre a caixa de cheats se estiver fechada, fecha se estiver aberta. */
	UFUNCTION(BlueprintCallable, Category = "Cheats")
	void ToggleCheatWidget();

	UFUNCTION(BlueprintCallable, Category = "Cheats")
	void OpenCheatWidget();

	UFUNCTION(BlueprintCallable, Category = "Cheats")
	void CloseCheatWidget();

	UFUNCTION(BlueprintPure, Category = "Cheats")
	bool IsCheatWidgetOpen() const;

	UFUNCTION(BlueprintCallable, Category = "Cheats")
	ECheatActivationResult RequestActivateCheatByCode(const FString& Code);

	/** True enquanto o controller troca o personagem via cheat (o personagem novo usa isso no spawn). */
	UFUNCTION(BlueprintPure, Category = "Cheats")
	bool IsSwappingCharacter() const { return bIsSwappingCharacter; }

protected:
	/** Widget da caixa de cheats (WBP_CheatCode). */
	UPROPERTY(EditDefaultsOnly, Category = "Cheats")
	TSubclassOf<UUserWidget> CheatWidgetClass;

	/** Input Action do atalho que abre/fecha a caixa. */
	UPROPERTY(EditDefaultsOnly, Category = "Cheats|Input")
	TObjectPtr<UInputAction> ToggleCheatAction;

	/** Mapping Context so com o atalho de cheats (separado do contexto do personagem). */
	UPROPERTY(EditDefaultsOnly, Category = "Cheats|Input")
	TObjectPtr<UInputMappingContext> CheatMappingContext;

	/** Prioridade do contexto acima do contexto do personagem (que usa 0). */
	UPROPERTY(EditDefaultsOnly, Category = "Cheats|Input")
	int32 CheatMappingPriority = 10;

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	UPROPERTY()
	TObjectPtr<UUserWidget> CheatWidgetInstance;

	/** Estado do cursor antes de abrir a caixa, pra restaurar ao fechar (menu x gameplay). */
	bool bCursorWasVisible = false;

	bool bIsSwappingCharacter = false;

	/**
	 * Servidor: destroi o personagem atual e faz respawn na mesma posicao.
	 * O GameMode escolhe a classe nova (cheat de personagem) e o inventario eh transferido.
	 */
	bool SwapCharacterInPlace();

	UCheatSubsystem* GetCheatSubsystem() const;

	/** Roda no servidor: ativa no subsystem e aplica no pawn deste jogador. */
	ECheatActivationResult ActivateAndApply(const FString& Code);

	UFUNCTION(Server, Reliable)
	void ServerActivateCheatByCode(const FString& Code);
};
