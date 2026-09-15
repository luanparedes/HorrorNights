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
	if (InController != nullptr && InController->PlayerState != nullptr)
	{
		ACharacterState* characterState = Cast<ACharacterState>(InController->PlayerState);

		if (characterState != nullptr)
		{
			if (characterState->SortedCharacter != nullptr)
			{
				if (characterState->SortedCharacter->CharacterBlueprint != nullptr)
				{
					return characterState->SortedCharacter->CharacterBlueprint;
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("There's no character blueprint configured for the sorted character!"));
				}
			}
		}
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

void AHorrorNightGameMode::PostLogin(APlayerController* NewPlayer)
{
	if (NewPlayer != nullptr && NewPlayer->PlayerState != nullptr)
	{
		ACharacterState* characterState = Cast<ACharacterState>(NewPlayer->PlayerState);

		if (characterState != nullptr && AvailableClasses.Num() > 0)
		{
			int32 sortedIndex = FMath::RandRange(0, AvailableClasses.Num() - 1);
			characterState->SortedCharacter = AvailableClasses[sortedIndex];

			UE_LOG(LogTemp, Warning, TEXT("Sorted player with index: %d"), sortedIndex);
		}
	}

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