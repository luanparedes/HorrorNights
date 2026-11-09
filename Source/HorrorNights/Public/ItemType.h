// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "ItemType.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Food			UMETA(DisplayName = "Food"),
	Materials	    UMETA(DisplayName = "Materials"),
	Recovery        UMETA(DisplayName = "Recovery"),
	Upgrade         UMETA(DisplayName = "Upgrade"),
	KeyItems        UMETA(DisplayName = "Key Items")
};

UENUM(BlueprintType)
enum class EFoodItems : uint8
{
	SmallBeef		UMETA(DisplayName = "Small Beef"),
	MediumBeef		UMETA(DisplayName = "Medium Beef"),
	LargeBeef		UMETA(DisplayName = "Large Beef"),
	Apple			UMETA(DisplayName = "Apple"),
	Banana			UMETA(DisplayName = "Banana"),
	Watermelon		UMETA(DisplayName = "Watermelon"),
	Bread			UMETA(DisplayName = "Bread"),
	Pizza			UMETA(DisplayName = "Pizza"),
};

UENUM(BlueprintType)
enum class EMaterialsItems : uint8
{
	Wood		UMETA(DisplayName = "Wood"),
	Metal		UMETA(DisplayName = "Metal"),
	Stone		UMETA(DisplayName = "Stone"),
	Book		UMETA(DisplayName = "Book"),
	Seasoning	UMETA(DisplayName = "Seasoning"),
	Beef		UMETA(DisplayName = "Beef"),
};

UENUM(BlueprintType)
enum class ERecoveryItems : uint8
{
	Potion100		UMETA(DisplayName = "Potion +100"),
	Potion300		UMETA(DisplayName = "Potion +300"),
	Potion500		UMETA(DisplayName = "Potion +500"),
	Poison100		UMETA(DisplayName = "Poison +100"),
};

UENUM(BlueprintType)
enum class EUpgradeItems : uint8
{
	Attack100		UMETA(DisplayName = "Attack +100"),
	Attack300		UMETA(DisplayName = "Attack +300"),
	Attack500		UMETA(DisplayName = "Attack +500"),
	Defense100		UMETA(DisplayName = "Defense +100"),
	Defense300		UMETA(DisplayName = "Defense +300"),
	Defense500		UMETA(DisplayName = "Defense +500"),
	Magic100		UMETA(DisplayName = "Magic +100"),
	Magic300		UMETA(DisplayName = "Magic +300"),
	Magic500		UMETA(DisplayName = "Magic +500"),
	Speed100		UMETA(DisplayName = "Speed +100"),
	Speed300		UMETA(DisplayName = "Speed +300"),
	Speed500		UMETA(DisplayName = "Speed +500"),
	Smart100		UMETA(DisplayName = "Smart +100"),
	Smart300		UMETA(DisplayName = "Smart +300"),
	Smart500		UMETA(DisplayName = "Smart +500"),
	MaxHP300		UMETA(DisplayName = "Max HP +300"),
	MaxHP500		UMETA(DisplayName = "Max HP +500"),
	MaxHP1000		UMETA(DisplayName = "Max HP +1000"),
};

UENUM(BlueprintType)
enum class EKeyItems : uint8
{
	BlueKey			UMETA(DisplayName = "Blue Key"),
	RedKey			UMETA(DisplayName = "Red Key"),
	GreenKey		UMETA(DisplayName = "Green Key"),
	YellowKey		UMETA(DisplayName = "Yellow Key"),
	WhiteKey		UMETA(DisplayName = "White Key"),
	BlackKey		UMETA(DisplayName = "Black Key"),
};