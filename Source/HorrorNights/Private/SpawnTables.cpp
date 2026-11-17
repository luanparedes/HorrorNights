// Sunnymoon Softwares - All rights reserved


#include "SpawnTables.h"
#include "EnemyCharacter.h"
#include "AnimalCharacter.h"
#include "Item.h"

namespace
{
	template <typename TEntry, typename TPredicate>
	int32 PickWeightedIndex(const TArray<TEntry>& Entries, TPredicate IsEligible)
	{
		float TotalWeight = 0.f;
		int32 LastEligible = INDEX_NONE;

		for (int32 Index = 0; Index < Entries.Num(); ++Index)
		{
			if (Entries[Index].Weight > 0.f && IsEligible(Entries[Index]))
			{
				TotalWeight += Entries[Index].Weight;
				LastEligible = Index;
			}
		}

		if (LastEligible == INDEX_NONE)
		{
			return INDEX_NONE;
		}

		float Roll = FMath::FRandRange(0.f, TotalWeight);

		for (int32 Index = 0; Index < Entries.Num(); ++Index)
		{
			if (Entries[Index].Weight > 0.f && IsEligible(Entries[Index]))
			{
				Roll -= Entries[Index].Weight;

				if (Roll <= 0.f)
				{
					return Index;
				}
			}
		}

		return LastEligible;
	}
}

bool FMonsterSpawnEntry::PickPower(EEnemyPower& OutPower) const
{
	const int32 Index = PickWeightedIndex(PowerWeights, [](const FEnemyPowerWeight&) { return true; });

	if (Index == INDEX_NONE)
	{
		return false;
	}

	OutPower = PowerWeights[Index].Power;
	return true;
}

int32 UMonsterSpawnTable::GetCountForNight(int32 NightNumber) const
{
	const int32 NightsPassed = FMath::Max(NightNumber - 1, 0);
	const float Count = static_cast<float>(BaseCount + ExtraCountPerNight * NightsPassed);
	const float Varied = Count * (1.f + FMath::FRandRange(-CountVariance, CountVariance));

	return FMath::Clamp(FMath::RoundToInt(Varied), 0, MaxCount);
}

int32 UMonsterSpawnTable::PickEntryIndex(int32 NightNumber) const
{
	return PickWeightedIndex(Entries, [NightNumber](const FMonsterSpawnEntry& Entry)
	{
		return Entry.MonsterClass != nullptr && Entry.FirstNight <= NightNumber;
	});
}

int32 UItemSpawnTable::GetCountForDay() const
{
	return FMath::RandRange(FMath::Min(MinCount, MaxCount), FMath::Max(MinCount, MaxCount));
}

int32 PickWeightedItemEntry(const TArray<FItemSpawnEntry>& Entries)
{
	return PickWeightedIndex(Entries, [](const FItemSpawnEntry& Entry)
	{
		return Entry.ItemClass != nullptr;
	});
}

int32 UItemSpawnTable::PickEntryIndex() const
{
	return PickWeightedItemEntry(Entries);
}

int32 UAnimalSpawnTable::GetTargetCount() const
{
	return FMath::RandRange(FMath::Min(MinCount, MaxCount), FMath::Max(MinCount, MaxCount));
}

int32 UAnimalSpawnTable::PickEntryIndex() const
{
	return PickWeightedIndex(Entries, [](const FAnimalSpawnEntry& Entry)
	{
		return Entry.AnimalClass != nullptr;
	});
}
