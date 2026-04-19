//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

namespace b3d
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Memory-Internal
	 *  @{
	 */

	/**
	 * Specialized allocator for profiler so we can avoid tracking internal profiler memory allocations which would skew
	 * profiler results.
	 */
	class ProfilerAllocatorTag
	{};

	/** Memory allocator providing a generic implementation. Specialize for specific categories as needed. */
	template <>
	class MemoryAllocator<ProfilerAllocatorTag> : public MemoryAllocatorBase
	{
	public:
		/** Allocates the given number of bytes. */
		static void* Allocate(size_t bytes)
		{
			return malloc(bytes);
		}

		/** Frees memory previously allocated with allocate(). */
		static void Free(void* ptr)
		{
			::free(ptr);
		}
	};

	/** @} */
	/** @} */
} // namespace b3d
