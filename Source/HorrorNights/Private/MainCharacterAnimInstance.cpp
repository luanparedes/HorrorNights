// Sunnymoon Softwares - All rights reserved

#include "MainCharacterAnimInstance.h"
#include "PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UMainCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<APlayerCharacter>(TryGetPawnOwner());

	if (Character)
	{
		MovementComponent = Character->GetCharacterMovement();
	}
}

void UMainCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (MovementComponent)
	{
		GroundSpeed = UKismetMathLibrary::VSizeXY(MovementComponent->Velocity);
		IsFalling = MovementComponent->IsFalling();
	}
}