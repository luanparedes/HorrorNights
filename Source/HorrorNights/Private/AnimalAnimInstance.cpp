// Sunnymoon Softwares - All rights reserved


#include "AnimalAnimInstance.h"
#include "AnimalCharacter.h"
#include "AnimalClassData.h"
#include "GameFramework/CharacterMovementComponent.h"

void UAnimalAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Animal = Cast<AAnimalCharacter>(TryGetPawnOwner());
	bHasLastYaw = false;
}

void UAnimalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!Animal)
	{
		Animal = Cast<AAnimalCharacter>(TryGetPawnOwner());
		if (!Animal)
		{
			return;
		}
	}

	if (const UAnimalClassData* Data = Animal->ClassData)
	{
		WalkSpeed = FMath::Max(Data->WalkSpeed, 1.f);
		RunSpeed = FMath::Max(Data->RunSpeed, WalkSpeed + 1.f);
	}

	const FVector Velocity = Animal->GetVelocity();
	GroundSpeed = Velocity.Size2D();

	bIsDead = Animal->IsDead();
	bIsMoving = !bIsDead && GroundSpeed > MovingThreshold;
	bIsRunning = bIsMoving && GroundSpeed > (WalkSpeed + RunSpeed) * 0.5f;

	if (const UCharacterMovementComponent* Movement = Animal->GetCharacterMovement())
	{
		bIsFalling = Movement->IsFalling();
	}

	float TargetBlend = 0.f;
	if (bIsMoving)
	{
		TargetBlend = GroundSpeed <= WalkSpeed
			? GroundSpeed / WalkSpeed
			: 1.f + FMath::Clamp((GroundSpeed - WalkSpeed) / (RunSpeed - WalkSpeed), 0.f, 1.f);
	}

	LocomotionBlend = LocomotionInterpSpeed > 0.f
		? FMath::FInterpTo(LocomotionBlend, TargetBlend, DeltaSeconds, LocomotionInterpSpeed)
		: TargetBlend;

	const FRotator Rotation = Animal->GetActorRotation();

	Direction = 0.f;
	if (bIsMoving)
	{
		const FVector LocalVelocity = Rotation.UnrotateVector(Velocity);
		Direction = FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X));
	}

	if (bHasLastYaw && DeltaSeconds > KINDA_SMALL_NUMBER)
	{
		const float RawTurnRate = FMath::FindDeltaAngleDegrees(LastYaw, Rotation.Yaw) / DeltaSeconds;
		TurnRate = FMath::FInterpTo(TurnRate, RawTurnRate, DeltaSeconds, 8.f);
	}
	LastYaw = Rotation.Yaw;
	bHasLastYaw = true;
}
