//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
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
	 * Free allocator with no limitations, using traditional malloc/free under the hood. */
	class FreeAlloc
	{
	public:
		/** Allocates memory. */
		u8* Alloc(u32 amount)
		{
			return (u8*)malloc(amount);
		}

		/** Deallocates a previously allocated piece of memory. */
		void Free(void* data)
		{
			::free(data);
		}

		/** Unused */
		void Clear()
		{
			// Do nothing
		}
	};

	/** @} */
	/** @} */
} // namespace b3d
