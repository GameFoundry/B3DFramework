//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "Allocators/BsPoolAlloc.h"

namespace bs { namespace ct
{
	/** @addtogroup RenderBeast
	 *  @{
	 */

	/** Information about an allocation in a single row of a texture. */
	struct TextureRowAllocation
	{
		/** Starting pixels of the allocation. */
		uint16_t x = 0, y = 0;

		/** Number of pixels in the allocation. */
		uint32_t length = 0;
	};

	/** Allocates elements of variable size within rows of a texture. */
	template<uint32_t WIDTH, uint32_t HEIGHT>
	class TextureRowAllocator : public INonCopyable
	{
	public:
		TextureRowAllocator();
		~TextureRowAllocator();

		/**
		 * Attempts to Allocate a new region of size @p length pixels. Returned allocation will have the same size as the
		 * requested size if sucessful.
		 */
		TextureRowAllocation Alloc(uint32_t length);

		/** Frees a previously allocated region. */
		void Free(const TextureRowAllocation& alloc);

	private:
		/** Describes a single contigous region of a texture row. */
		struct RowRegion
		{
			// Note: 'next' must be the first member because of shenanigans we do in alloc() and free()
			RowRegion* next = nullptr;
			uint32_t x = 0;
			uint32_t length = WIDTH;
		};

		RowRegion* mFreeRegions[HEIGHT];
		PoolAlloc<sizeof(RowRegion), HEIGHT * 2> mAlloc;
	};

	template <uint32_t WIDTH, uint32_t HEIGHT>
	TextureRowAllocator<WIDTH, HEIGHT>::TextureRowAllocator()
	{
		for(uint32_t i = 0; i < HEIGHT; i++)
			mFreeRegions[i] = mAlloc.template Construct<RowRegion>();
	}

	template <uint32_t WIDTH, uint32_t HEIGHT>
	TextureRowAllocator<WIDTH, HEIGHT>::~TextureRowAllocator()
	{
		for(uint32_t i = 0; i < HEIGHT; i++)
		{
			RowRegion* region = mFreeRegions[i];
			while(region)
			{
				RowRegion* curRegion = region;
				region = region->next;

				mAlloc.Free(curRegion);
			}
		}
	}

	template<uint32_t WIDTH, uint32_t HEIGHT>
	TextureRowAllocation TextureRowAllocator<WIDTH, HEIGHT>::alloc(uint32_t length)
	{
		TextureRowAllocation output;

		for(uint32_t i = 0; i < HEIGHT; i++)
		{
			RowRegion* region = mFreeRegions[i];
			RowRegion* prevRegion = (RowRegion*)&mFreeRegions[i]; // This ensures an assignment to prevRegion->next changes the entry of mFreeRegions
			while(region)
			{
				if(region->length == length)
				{
					output.x = region->x;
					output.y = i;
					output.length = length;

					prevRegion->next = region->next;
					mAlloc.Free(region);

					return output;
				}

				if(region->length > length)
				{
					output.x = region->x;
					output.y = i;
					output.length = length;

					region->x += length;
					region->length -= length;

					return output;
				}

				prevRegion = region;
				region = region->next;
			}
		}

		return output;
	}

	template <uint32_t WIDTH, uint32_t HEIGHT>
	void TextureRowAllocator<WIDTH, HEIGHT>::free(const TextureRowAllocation& alloc)
	{
		if(alloc.length == 0)
			return;

		RowRegion* region = mFreeRegions[alloc.y];
		RowRegion* prevRegion = (RowRegion*)&mFreeRegions[alloc.y]; // This ensures an assignment to prevRegion->next changes the entry of mFreeRegions

		if(region)
		{
			// Find the location where to insert the free region
			while (region && alloc.x > (region->x + region->length))
			{
				prevRegion = region;
				region = region->next;
			}

			if(region)
			{
				// End of the allocation is the beginning of this region
				if((alloc.x + alloc.length) == region->x)
				{
					region->x -= alloc.length;
					region->length += alloc.length;

					return;
				}

				// Beginning of the allocation is at the end of this region
				const uint32_t regionEnd = region->x + region->length;
				if (alloc.x == regionEnd)
				{
					region->length += alloc.length;

					// Merge any directly following regions
					prevRegion = region;
					region = region->next;

					while(region && region->x == (prevRegion->x + prevRegion->length))
					{
						prevRegion->length += region->length;
						prevRegion->next = region->next;

						RowRegion* toDelete = region;
						region = region->next;

						mAlloc.Free(toDelete);
					}

					return;
				}
			}
		}
			
		auto newRegion = (RowRegion*)mAlloc.Alloc();
		newRegion->x = alloc.x;
		newRegion->length = alloc.length;
		newRegion->next = region;

		prevRegion->next = newRegion;
	}

	/* @} */
}}
