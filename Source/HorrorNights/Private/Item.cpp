// Sunnymoon Softwares - All rights reserved


#include "Item.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "PlayerCharacter.h"
#include "InventoryComponent.h"
#include "NiagaraComponent.h"

AItem::AItem()
{
	PrimaryActorTick.bCanEverTick = false;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	RootComponent = ItemMesh;
	ItemMesh->SetCollisionProfileName(TEXT("NoCollision"));

	ItemEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ItemEffect"));
	ItemEffect->SetupAttachment(RootComponent);

	PickupZone = CreateDefaultSubobject<USphereComponent>(TEXT("PickupZone"));
	PickupZone->SetupAttachment(RootComponent);
	PickupZone->SetSphereRadius(80.f);
	PickupZone->SetCollisionProfileName(TEXT("Trigger"));
	PickupZone->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	PickupZone->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap);

	PickupZone->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnOverlapBegin);
	PickupZone->OnComponentEndOverlap.AddDynamic(this, &AItem::OnOverlapEnd);

	OverlappingPlayer = nullptr;
}

void AItem::BeginPlay()
{
	Super::BeginPlay();
	SnapToGround();
}

void AItem::Interact(APlayerCharacter* PlayerCalling)
{
	if (PlayerCalling && PlayerCalling->Inventory && PlayerCalling->ItemOnRange && ItemData)
	{
		// Adiciona no inventário
		PlayerCalling->Inventory->AddItem(ItemData, Quantity);

		// Destrói o item do mundo
		Destroy();
	}
}

void AItem::SnapToGround()
{
	FVector StartLocation = GetActorLocation();
	FVector EndLocation = StartLocation - FVector(0.f, 0.f, 1000.f);

	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, CollisionParams))
	{
		SetActorLocation(HitResult.ImpactPoint);
	}
}

void AItem::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (ItemMesh)
	{
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ItemMesh->SetCollisionProfileName(TEXT("NoCollision"));
	}
}

void AItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this)
	{
		if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
		{
			Player->ItemOnRange = this;
			OverlappingPlayer = Player;
			// Aqui no futuro você pode ativar um Widget na tela do player dizendo: "Pressione E para pegar"
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("Pressione o botão de ação para pegar o item!"));
		}
	}
}

void AItem::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor == OverlappingPlayer)
	{
		if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
		{
			Player->ItemOnRange = nullptr;
			OverlappingPlayer = nullptr;
		}
	}
}
