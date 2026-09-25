// Sunnymoon Softwares - All rights reserved


#include "PlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CharacterState.h"
#include "Net/UnrealNetwork.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera Component"));
	CameraComp->SetupAttachment(CameraBoom);

	PlayerStats = CreateDefaultSubobject<UPlayerStatsComponent>(TEXT("PlayerStats"));
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (APlayerController* playerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(playerController->GetLocalPlayer()))
		{
			subsystem->AddMappingContext(InputMapping, 0);
		}
	}
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	ACharacterState* PS = GetPlayerState<ACharacterState>();

	if (PS != nullptr && PS->SortedCharacter != nullptr)
	{
		if (PlayerStats != nullptr)
		{
			// Transfere a ficha do RG pro componente e inicializa
			PlayerStats->CurrentClassData = PS->SortedCharacter;
			PlayerStats->InitializeStats();

			// Agora sim, é seguro setar a velocidade e pulo!
			GetPlayerInitialStats();
		}
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D movementVector = Value.Get<FVector2D>();

	const FRotator rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, rotation.Yaw, 0.f);

	const FVector forwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector rightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(forwardDirection, movementVector.Y);
	AddMovementInput(rightDirection, movementVector.X);
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D lookAxisVector = Value.Get<FVector2D>();

	AddControllerPitchInput(lookAxisVector.Y);
	AddControllerYawInput(lookAxisVector.X);
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* enhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		enhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		enhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		enhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
	}
}

void APlayerCharacter::GetPlayerInitialStats()
{
	// 1. Regra de ouro: Checar se o componente e a ficha não são nulos antes de ler!
	if (PlayerStats != nullptr && PlayerStats->CurrentClassData != nullptr)
	{
		UCharacterMovementComponent* MoveComp = GetCharacterMovement();

		if (MoveComp != nullptr)
		{
			// 2. Ajusta a velocidade. 
			// Como o MaxWalkSpeed espera um float e sua ficha tem um int32, fazemos um cast básico
			MoveComp->MaxWalkSpeed = static_cast<float>(PlayerStats->CurrentClassData->Speed);

			// 3. Ajusta a força do pulo (Z é o eixo para cima na Unreal)
			MoveComp->JumpZVelocity = static_cast<float>(PlayerStats->CurrentClassData->Jump);

			UE_LOG(LogTemp, Warning, TEXT("Status aplicados com sucesso! Speed: %f | Jump: %f"), MoveComp->MaxWalkSpeed, MoveComp->JumpZVelocity);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Opa! PlayerStats ou a Ficha estao nulos ao tentar setar os status iniciais."));
	}
}

void APlayerCharacter::ServerRPCFunction_Implementation()
{
	if (!HasAuthority())
	{
		// Client-side sending RPC to server
	}
}