//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Math/B3DVector3I.h"

namespace b3d
{
	struct IGpuHeap;
	class IGpuResource;

	/** @addtogroup GpuBackend
	 *  @{
	 */

	/** Size of a page of virtual GPU memory, in bytes. The same on every device. */
	constexpr u32 kGpuPageSize = 64 * 1024;

	/** Consecutive pages, of a virtual texture or buffer or of a heap. */
	struct GpuPageRange
	{
		GpuPageRange() = default;
		GpuPageRange(u32 firstPageIndex, u32 pageCount)
			: FirstPageIndex(firstPageIndex), PageCount(pageCount)
		{ }

		/** Returns the index of the page after the last page of the range. */
		u32 GetEndPageIndex() const { return FirstPageIndex + PageCount; }

		u32 FirstPageIndex = 0;
		u32 PageCount = 0;
	};

	/** Consecutive pages of memory in a heap. */
	struct GpuPageAllocation
	{
		GpuPageAllocation() = default;
		GpuPageAllocation(IGpuHeap* heap, const GpuPageRange& pages)
			: Heap(heap), Pages(pages)
		{ }

		IGpuHeap* Heap = nullptr; /**< Null for no memory. */
		GpuPageRange Pages; /**< Pages of Heap. */
	};

	/** Heap memory that a page range of a virtual buffer or texture maps to, replacing the memory it mapped to before. */
	struct GpuPageMapping
	{
		GpuPageMapping() = default;
		GpuPageMapping(IGpuResource* resource, const GpuPageRange& pages, IGpuHeap* heap, u32 firstHeapPageIndex)
			: Resource(resource), Pages(pages), Heap(heap), FirstHeapPageIndex(firstHeapPageIndex)
		{ }

		IGpuResource* Resource = nullptr;
		GpuPageRange Pages; /**< Pages of Resource. */
		IGpuHeap* Heap = nullptr; /**< Heap whose pages become the memory of Pages. Null unmaps them. */
		u32 FirstHeapPageIndex = 0; /**< Index of the page of Heap that the first page of Pages maps to. */
	};

	/**
	 * How the pages of a virtual texture map to its mip levels, array layers and texels.
	 *
	 * Pages are in this order: the shared tail if there is one, then layer by layer. Within a layer the tail comes first, then the mip levels
	 * from the smallest to the largest. Within a mip level, tiles are in raster order: by depth slice, then row, then column. So the pages a
	 * mip chain needs are one consecutive range per layer, and streaming a mip level in or out commits or decommits the pages next to them.
	 */
	struct B3D_EXPORT GpuVirtualTextureLayout
	{
		Vector3I Size; /**< Texels of mip level 0. */
		u32 MipLevelCount = 0; /**< Number of mip levels, the tail included. */
		Vector3I TileSize; /**< Texels one page covers in a mip level outside the tail. */
		u32 FirstTailMipLevel = 0; /**< This mip level and all smaller ones share the tail pages. Equals MipLevelCount if there is no tail. */
		u32 TailPageCount = 0;
		bool HasSharedTail = false; /**< One tail for all array layers, instead of one per layer. */
		u32 PageCountPerLayer = 0; /**< Pages of one array layer, without the shared tail. */
		u32 PageCount = 0;

		/** Sets PageCountPerLayer and PageCount from the other fields, for a texture with @p arrayLayerCount layers. */
		void UpdatePageCounts(u32 arrayLayerCount);

		/** Returns the number of tiles of a mip level outside the tail, along each axis. */
		Vector3I GetTileCount(u32 mipLevel) const;

		/** Returns the pages of one mip level of one array layer. For a tail mip level, the tail pages. */
		GpuPageRange GetMipLevelPages(u32 arrayLayer, u32 mipLevel) const;

		/**
		 * Returns the pages of all mip levels from @p firstMipLevel to the smallest, of one array layer. One consecutive range. A shared tail
		 * is not part of the range unless @p firstMipLevel is a tail mip level, see GetMipLevelPages().
		 */
		GpuPageRange GetMipChainPages(u32 arrayLayer, u32 firstMipLevel) const;

		/** Returns the page that holds one tile of a mip level outside the tail. */
		u32 GetPageIndex(u32 arrayLayer, u32 mipLevel, const Vector3I& tile) const;
	};

	/** Flags that describe the resource of a GpuPageTable. */
	enum class GpuPageTableFlag
	{
		None = 0,

		/** The resource has a bindless slot, so any command buffer that binds a bindless heap may read it. */
		Bindless = 1 << 0
	};

	using GpuPageTableFlags = Flags<GpuPageTableFlag>;
	B3D_FLAGS_OPERATORS(GpuPageTableFlag);

	/** @} */
} // namespace b3d
