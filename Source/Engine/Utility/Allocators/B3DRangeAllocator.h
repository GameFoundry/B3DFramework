//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Memory-Internal
	 *  @{
	 */

	/**
	 * Allocates ranges of consecutive indices out of a fixed number of them, such as the slots of a GPU buffer or the descriptors
	 * of a descriptor heap. Freed ranges merge with the free ranges next to them.
	 *
	 * @note	Not thread safe.
	 */
	class B3D_EXPORT RangeAllocator
	{
	public:
		/** Returned by Allocate() when no free range is large enough. */
		static constexpr u32 kInvalidIndex = ~0u;

		explicit RangeAllocator(u32 capacity = 0);

		/** Allocates @p count consecutive indices, and returns the first, or kInvalidIndex if no free range is large enough. */
		u32 Allocate(u32 count);

		/** Frees a range returned by Allocate(). */
		void Free(u32 first, u32 count);

	private:
		/** Indices that are not allocated. */
		struct FreeRange
		{
			u32 First = 0;
			u32 Count = 0;
		};

		/** Ordered by first index, and never adjacent to each other. */
		TArray<FreeRange> mFreeRanges;
	};

	/** @} */
	/** @} */
} // namespace b3d
