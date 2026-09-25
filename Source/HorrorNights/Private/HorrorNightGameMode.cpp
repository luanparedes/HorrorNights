// Sunnymoon Softwares - All rights reserved


#include "HorrorNightGameMode.h"
#include "CharacterState.h"
#include "PlayerClassData.h"
#include "RHI.h"
#include "RenderQualityInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

void AHorrorNightGameMode::BeginPlay()
{
	Super::BeginPlay();
	ConfigureGraphics();
}

UClass* AHorrorNightGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	UE_LOG(LogTemp, Warning, TEXT(">>> GetDefaultPawnClass FOI CHAMADA <<<"));

	if (InController == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Vazou de fininho: Controller eh nulo!"));
		return Super::GetDefaultPawnClassForController_Implementation(InController);
	}

	if (InController->PlayerState == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Vazou de fininho: PlayerState eh nulo!"));
		return Super::GetDefaultPawnClassForController_Implementation(InController);
	}

	ACharacterState* characterState = Cast<ACharacterState>(InController->PlayerState);
	if (characterState == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Vazou de fininho: O Cast pro seu CharacterState falhou!"));
		return Super::GetDefaultPawnClassForController_Implementation(InController);
	}

	if (characterState->SortedCharacter == nullptr)
	{
		if (AvailableClasses.Num() > 0)
		{
			int32 sortedIndex = FMath::RandRange(0, AvailableClasses.Num() - 1);
			characterState->SortedCharacter = AvailableClasses[sortedIndex];
			UE_LOG(LogTemp, Warning, TEXT("Sorteio feito na hora H! Personagem Index: %d"), sortedIndex);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Erro: A lista AvailableClasses ta VAZIA!"));
			return Super::GetDefaultPawnClassForController_Implementation(InController);
		}
	}

	if (characterState->SortedCharacter->CharacterBlueprint != nullptr)
	{
		FString NomeDoBP = characterState->SortedCharacter->CharacterBlueprint->GetName();
		UE_LOG(LogTemp, Warning, TEXT("SUCCESS loading player: %s"), *NomeDoBP);

		return characterState->SortedCharacter->CharacterBlueprint;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Erro: O Data Asset sorteado nao tem um CharacterBlueprint configurado nele."));
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

void AHorrorNightGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
}

void AHorrorNightGameMode::ConfigureGraphics()
{
	const bool bHasSM5 = GMaxRHIFeatureLevel >= ERHIFeatureLevel::SM5;

	if (bHasSM5)
	{
        UE_LOG(LogTemp, Log, TEXT("Rendering: SM5 detected - using advanced pipeline"));
		SetupHighEndRendering();
	}
	else
	{
        UE_LOG(LogTemp, Log, TEXT("Rendering: fallback ES3.1 - using lower mobile pipeline."));
		SetupFallbackRendering();
	}
}

void AHorrorNightGameMode::SetupHighEndRendering()
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithInterface(GetWorld(), URenderQualityInterface::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        IRenderQualityInterface::Execute_OnHighEndRenderingEnabled(Actor);
    }
}

void AHorrorNightGameMode::SetupFallbackRendering()
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithInterface(GetWorld(), URenderQualityInterface::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        IRenderQualityInterface::Execute_OnFallbackRenderingEnabled(Actor);
    }
}