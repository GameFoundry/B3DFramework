//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "Allocators/B3DPoolAlloc.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup RenderBeast
		 *  @{
		 */

		/** Information about an allocation in a single row of a texture. */
		struct TextureRowAllocation
		{
			/** Starting pixels of the allocation. */
			uint16_t X = 0, Y = 0;

			/** Number of pixels in the allocation. */
			uint32_t Length = 0;
		};

		/** Allocates elements of variable size within rows of a texture. */
		template <uint32_t WIDTH, uint32_t HEIGHT>
		class TextureRowAllocator : public INonCopyable
		{
		public:
			TextureRowAllocator();
			~TextureRowAllocator();

			/**
			 * Attempts to allocate a new region of size @p length pixels. Returned allocation will have the same size as the
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
				RowRegion* Next = nullptr;
				uint32_t X = 0;
				uint32_t Length = WIDTH;
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
					region = region->Next;

					mAlloc.Free(curRegion);
				}
			}
		}

		template <uint32_t WIDTH, uint32_t HEIGHT>
		TextureRowAllocation TextureRowAllocator<WIDTH, HEIGHT>::Alloc(uint32_t length)
		{
			TextureRowAllocation output;

			for(uint32_t i = 0; i < HEIGHT; i++)
			{
				RowRegion* region = mFreeRegions[i];
				RowRegion* prevRegion = (RowRegion*)&mFreeRegions[i]; // This ensures an assignment to prevRegion->next changes the entry of mFreeRegions
				while(region)
				{
					if(region->Length == length)
					{
						output.X = region->X;
						output.Y = i;
						output.Length = length;

						prevRegion->Next = region->Next;
						mAlloc.Free(region);

						return output;
					}

					if(region->Length > length)
					{
						output.X = region->X;
						output.Y = i;
						output.Length = length;

						region->X += length;
						region->Length -= length;

						return output;
					}

					prevRegion = region;
					region = region->Next;
				}
			}

			return output;
		}

		template <uint32_t WIDTH, uint32_t HEIGHT>
		void TextureRowAllocator<WIDTH, HEIGHT>::Free(const TextureRowAllocation& alloc)
		{
			if(alloc.Length == 0)
				return;

			RowRegion* region = mFreeRegions[alloc.Y];
			RowRegion* prevRegion = (RowRegion*)&mFreeRegions[alloc.Y]; // This ensures an assignment to prevRegion->next changes the entry of mFreeRegions

			if(region)
			{
				// Find the location where to insert the free region
				while(region && alloc.X > (region->X + region->Length))
				{
					prevRegion = region;
					region = region->Next;
				}

				if(region)
				{
					// End of the allocation is the beginning of this region
					if((alloc.X + alloc.Length) == region->X)
					{
						region->X -= alloc.Length;
						region->Length += alloc.Length;

						return;
					}

					// Beginning of the allocation is at the end of this region
					const uint32_t regionEnd = region->X + region->Length;
					if(alloc.X == regionEnd)
					{
						region->Length += alloc.Length;

						// Merge any directly following regions
						prevRegion = region;
						region = region->Next;

						while(region && region->X == (prevRegion->X + prevRegion->Length))
						{
							prevRegion->Length += region->Length;
							prevRegion->Next = region->Next;

							RowRegion* toDelete = region;
							region = region->Next;

							mAlloc.Free(toDelete);
						}

						return;
					}
				}
			}

			auto newRegion = (RowRegion*)mAlloc.Alloc();
			newRegion->X = alloc.X;
			newRegion->Length = alloc.Length;
			newRegion->Next = region;

			prevRegion->Next = newRegion;
		}

		/* @} */
	} // namespace render
} // namespace b3d
