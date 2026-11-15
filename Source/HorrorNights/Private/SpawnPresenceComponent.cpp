// Sunnymoon Softwares - All rights reserved


#include "SpawnPresenceComponent.h"
#include "Components/MeshComponent.h"

USpawnPresenceComponent::USpawnPresenceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void USpawnPresenceComponent::PlayAppear()
{
	bVanishing = false;
	OnAppearStarted.Broadcast();

	if (FadeInDuration <= 0.f)
	{
		FadeAlpha = 1.f;
		ApplyFade(FadeAlpha);
		return;
	}

	FadeAlpha = 0.f;
	FadeTarget = 1.f;
	FadeSpeed = 1.f / FadeInDuration;
	ApplyFade(FadeAlpha);
	SetComponentTickEnabled(true);
}

void USpawnPresenceComponent::PlayVanishAndDestroy()
{
	if (bVanishing)
	{
		return;
	}

	bVanishing = true;
	OnVanishStarted.Broadcast();

	if (FadeOutDuration <= 0.f)
	{
		GetOwner()->Destroy();
		return;
	}

	FadeTarget = 0.f;
	FadeSpeed = 1.f / FadeOutDuration;
	SetComponentTickEnabled(true);
}

void USpawnPresenceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FadeAlpha = FMath::FInterpConstantTo(FadeAlpha, FadeTarget, DeltaTime, FadeSpeed);
	ApplyFade(FadeAlpha);

	if (FMath::IsNearlyEqual(FadeAlpha, FadeTarget))
	{
		SetComponentTickEnabled(false);

		if (bVanishing)
		{
			GetOwner()->Destroy();
		}
	}
}

void USpawnPresenceComponent::ApplyFade(float Alpha) const
{
	if (AActor* Owner = GetOwner())
	{
		Owner->ForEachComponent<UMeshComponent>(false, [this, Alpha](UMeshComponent* Mesh)
		{
			Mesh->SetScalarParameterValueOnMaterials(FadeParameterName, Alpha);
		});
	}
}
