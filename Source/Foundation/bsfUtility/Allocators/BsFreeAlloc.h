//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

namespace bs
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
		UINT8* Allocate(UINT32 amount)
		{
			return (UINT8*)malloc(amount);
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
}
