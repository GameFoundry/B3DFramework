//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Allocators/B3DRangeAllocator.h"

using namespace b3d;

RangeAllocator::RangeAllocator(u32 capacity)
{
	if (capacity > 0)
		mFreeRanges.Add(FreeRange{ 0, capacity });
}

u32 RangeAllocator::Allocate(u32 count)
{
	if (count == 0)
		return kInvalidIndex;

	// First fit, so that ranges freed by short-lived users are reused before the rest
	for (auto iterator = mFreeRanges.Begin(); iterator != mFreeRanges.End(); ++iterator)
	{
		if (iterator->Count < count)
			continue;

		const u32 first = iterator->First;
		iterator->First += count;
		iterator->Count -= count;
		if (iterator->Count == 0)
			mFreeRanges.Erase(iterator);

		return first;
	}

	return kInvalidIndex;
}

void RangeAllocator::Free(u32 first, u32 count)
{
	if (count == 0)
		return;

	// Merges the range with the free ranges right before and after it
	auto next = std::lower_bound(mFreeRanges.Begin(), mFreeRanges.End(), first,
		[](const FreeRange& range, u32 index) { return range.First < index; });

	FreeRange freed{ first, count };
	if (next != mFreeRanges.End() && freed.First + freed.Count == next->First)
	{
		freed.Count += next->Count;
		next = mFreeRanges.Erase(next);
	}

	if (next != mFreeRanges.Begin())
	{
		FreeRange& previous = *(next - 1);
		if (previous.First + previous.Count == freed.First)
		{
			previous.Count += freed.Count;
			return;
		}
	}

	mFreeRanges.Insert(next, freed);
}
