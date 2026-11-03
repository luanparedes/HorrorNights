// Sunnymoon Softwares - All rights reserved

#include "PlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "CharacterState.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "PlayerAttributeSet.h"
#include "PlayerClassData.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Item.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera Component"));
	CameraComp->SetupAttachment(CameraBoom);

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	PlayerAttributes = CreateDefaultSubobject<UPlayerAttributeSet>(TEXT("PlayerAttributes"));

	Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
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

	if (InventoryWidgetClass)
	{
		InventoryWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), InventoryWidgetClass);
	}
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(PlayerAttributes->GetSpeedAttribute()).AddUObject(this, &APlayerCharacter::OnSpeedChanged);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(PlayerAttributes->GetJumpAttribute()).AddUObject(this, &APlayerCharacter::OnJumpChanged);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(PlayerAttributes->GetHPAttribute()).AddUObject(this, &APlayerCharacter::OnHPChanged);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(PlayerAttributes->GetEnergyAttribute()).AddUObject(this, &APlayerCharacter::OnEnergyChanged);
	}

	ACharacterState* PS = GetPlayerState<ACharacterState>();

	if (PS != nullptr && PS->SortedCharacter != nullptr)
	{
		if (PlayerAttributes)
		{
			// Status Base
			PlayerAttributes->InitHP(PS->SortedCharacter->HP);
			// Se o seu banco tiver uma coluna MaxHP específica, mude para PS->SortedCharacter->MaxHP
			PlayerAttributes->InitMaxHP(PS->SortedCharacter->HP);
			PlayerAttributes->InitEnergy(PS->SortedCharacter->Energy);
			PlayerAttributes->InitMaxEnergy(PS->SortedCharacter->MaxEnergy);

			// Atributos de Combate e Status
			PlayerAttributes->InitAttack(PS->SortedCharacter->Attack);
			PlayerAttributes->InitDefense(PS->SortedCharacter->Defense);
			PlayerAttributes->InitSmartness(PS->SortedCharacter->Smartness);
			PlayerAttributes->InitMagicAttack(PS->SortedCharacter->MagicAttack);

			// Movimentação
			PlayerAttributes->InitSpeed(PS->SortedCharacter->Speed);
			PlayerAttributes->InitJump(PS->SortedCharacter->Jump);

			// Multiplicadores (Tempo de magia, cura, cozinha e velocidade do Ford Focus)
			PlayerAttributes->InitWaitMagicTimeMultiplier(PS->SortedCharacter->WaitMagicTimeMultiplier);
			PlayerAttributes->InitHealTimeMultiplier(PS->SortedCharacter->HealTimeMultiplier);
			PlayerAttributes->InitCookingTimeMultiplier(PS->SortedCharacter->CookingTimeMultiplier);
			PlayerAttributes->InitDriveVelocityMultiplier(PS->SortedCharacter->DriveVelocityMultiplier);

			// IncomingDamage é um meta-atributo só pra calcular o dano no momento da porrada, então nasce zerado
			PlayerAttributes->InitIncomingDamage(0.0f);

			// FIX: Atualiza o componente de movimento manualmente na inicialização
			if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
			{
				MoveComp->MaxWalkSpeed = PS->SortedCharacter->Speed;
				MoveComp->JumpZVelocity = PS->SortedCharacter->Jump;
			}

			UE_LOG(LogTemp, Warning, TEXT("Status applied with success directly from SortedCharacter to GAS!"));
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

void APlayerCharacter::OpenInventory(const FInputActionValue& Value)
{
	if (!InventoryWidgetInstance) 
		return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) 
		return;

	bIsInventoryOpen = !bIsInventoryOpen;

	if (bIsInventoryOpen)
	{
		InventoryWidgetInstance->AddToViewport();

		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(InventoryWidgetInstance->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}
	else
	{
		InventoryWidgetInstance->RemoveFromParent();

		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(false);
	}
}

void APlayerCharacter::Interact(const FInputActionValue& Value)
{
	if (ItemOnRange)
	{
		ItemOnRange->Interact(this);
		ItemOnRange = nullptr;
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* enhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		enhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		enhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		enhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		enhancedInputComponent->BindAction(OpenInventoryAction, ETriggerEvent::Started, this, &APlayerCharacter::OpenInventory);
		enhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &APlayerCharacter::Interact);
	}
}

void APlayerCharacter::Attack()
{

}

void APlayerCharacter::Defense()
{

}

void APlayerCharacter::Magic()
{

}

void APlayerCharacter::Ability()
{

}

UAbilitySystemComponent* APlayerCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void APlayerCharacter::ApplyItemEffect(TSubclassOf<UGameplayEffect> EffectToApply)
{
	if (AbilitySystemComponent && EffectToApply)
	{
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddInstigator(this, this);

		AbilitySystemComponent->ApplyGameplayEffectToSelf(EffectToApply.GetDefaultObject(), 1.0f, EffectContext);
	}
}

void APlayerCharacter::ServerRPCFunction_Implementation()
{
	if (!HasAuthority())
	{
		// Client-side sending RPC to server
	}
}

void APlayerCharacter::OnHPChanged(const FOnAttributeChangeData& Data)
{
	//It must update UI or any other logic related to HP change
}

void APlayerCharacter::OnEnergyChanged(const FOnAttributeChangeData& Data)
{
	//It must update UI or any other logic related to Energy change
}

void APlayerCharacter::OnJumpChanged(const FOnAttributeChangeData& Data)
{
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->JumpZVelocity = Data.NewValue;
	}
}

void APlayerCharacter::OnSpeedChanged(const FOnAttributeChangeData& Data)
{
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = Data.NewValue;
	}
}