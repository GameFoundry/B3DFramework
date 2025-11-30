//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "RenderAPI/B3DGpuBufferPool.h"
#include "RenderAPI/B3DGpuParameterSet.h"

namespace b3d::render
{
	/** @addtogroup RenderBeast
	 *  @{
	 */

	/**
	 * Manages renderable uniform buffer allocations. Objects whose suballocations come from the same underlying GpuBuffer
	 * share a GpuParameterSet, to be bound at set index 1.
	 */
	class RenderableUniformBufferManager
	{
	public:
		/** Allocation result for a renderable. */
		struct RenderableAllocation
		{
			SPtr<GpuParameterSet> SharedParameterSet;

			GpuBufferSuballocation PerObjectSuballocation;
			u32 PerObjectDynamicOffsetIndex = 0;
		};

		/** Allocation result for a decal (includes DecalParamBuffer). */
		struct DecalAllocation : RenderableAllocation // TODO - The allocation result should return a union (or variant) of all possible extra sub-allocations. And there should only be one Allocate method that takes an enum indicating type of allocation needed.
		{
			GpuBufferSuballocation DecalSuballocation;
			u32 DecalDynamicOffsetIndex = 0;
		};

		RenderableUniformBufferManager() = default;
		~RenderableUniformBufferManager() = default;

		/**
		 * Initializes the manager. Must be called before use.
		 *
		 * @param device	GPU device for buffer creation.
		 */
		void Initialize(GpuDevice& device);

		/**
		 * Allocates per-object buffer for a renderable.
		 *
		 * @param layout	Pipeline parameter layout for creating the shared GpuParameterSet.
		 * @return			Allocation containing suballocation and shared parameter set.
		 */
		RenderableAllocation AllocateForRenderable(const SPtr<GpuPipelineParameterLayout>& layout);

		/**
		 * Allocates per-object buffers for a decal (PerObject + DecalParams).
		 *
		 * @param layout	Pipeline parameter layout for creating the shared GpuParameterSet.
		 * @return			Allocation containing suballocations and shared parameter set.
		 */
		DecalAllocation AllocateForDecal(const SPtr<GpuPipelineParameterLayout>& layout);

		/**
		 * Releases a renderable allocation back to the pool.
		 * @param allocation	Allocation to release.
		 */
		void Release(const RenderableAllocation& allocation);

		/**
		 * Releases a decal allocation back to the pool.
		 * @param allocation	Allocation to release.
		 */
		void Release(const DecalAllocation& allocation);

	private:
		static constexpr u32 kRenderableEntriesPerBuffer = 1024;
		static constexpr u32 kDecalEntriesPerBuffer = 256;

		/** Composite key for parameter set lookup: (PerObjectBuffer, DecalBuffer or nullptr). */
		using BufferKey = std::pair<GpuBuffer*, GpuBuffer*>;

		/** Hash functor for BufferKey. */
		struct BufferKeyHash
		{
			size_t operator()(const BufferKey& key) const
			{
				size_t hash = 0;
				B3DCombineHash(hash, key.first);
				B3DCombineHash(hash, key.second);

				return hash;
			}
		};

		/** Tracks GpuParameterSets shared by objects using the same underlying buffer combination. */
		struct BufferParameterSetEntry
		{
			SPtr<GpuParameterSet> ParameterSet;
			u32 RefCount = 0;
		};

		/**
		 * Gets or creates a shared GpuParameterSet for the given buffer combination.
		 *
		 * @param perObjectBuffer	The per-object uniform buffer.
		 * @param decalBuffer		The decal parameter buffer, or nullptr for non-decals.
		 * @param layout			Pipeline parameter layout.
		 * @return					Shared GpuParameterSet for set #1.
		 */
		SPtr<GpuParameterSet> GetOrCreateParameterSet(const SPtr<GpuBuffer>& perObjectBuffer, const SPtr<GpuBuffer>& decalBuffer, const SPtr<GpuPipelineParameterLayout>& layout);

		/**
		 * Decrements ref count for a buffer combination's shared parameter set, removing if zero.
		 *
		 * @param perObjectBuffer	The per-object uniform buffer.
		 * @param decalBuffer		The decal parameter buffer, or nullptr for non-decals.
		 */
		void ReleaseParameterSet(const SPtr<GpuBuffer>& perObjectBuffer, const SPtr<GpuBuffer>& decalBuffer);

		GpuBufferPool mRenderablePool;
		GpuBufferPool mDecalPool;
		UnorderedMap<BufferKey, BufferParameterSetEntry, BufferKeyHash> mParameterSetsByBuffer;
		GpuDevice* mDevice = nullptr;
	};

	/** @} */
}
