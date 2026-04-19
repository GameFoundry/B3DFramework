//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "RenderAPI/B3DGpuBufferPool.h"
#include "RenderAPI/B3DGpuParameterSet.h"

namespace b3d::render
{
	struct RenderState;
	struct DecalRenderState;
	struct ParticleRenderState;
	class DecalProxy;

	/** @addtogroup RenderBeast
	 *  @{
	 */

	/**
	 * Manages persistent uniform buffer allocations. Uniform buffers are suballocated from larger GpuBuffers, and all
	 * related suballocations are guaranteed to share the same parameter set. This allows you to bind the parameter
	 * set once and draw multiple objects that use different uniform buffers, while only modifying the dynamic
	 * buffer offsets to point to the correct suballocation.
	 */
	class UniformBufferPools
	{
	public:
		/** Type of buffer pool. */
		enum BufferType : u8
		{
			PerObjectBuffer = 0, /**< Per-object transform data. */
			DecalBuffer = 1, /**< Decal-specific parameters. */
			GpuParticlesBuffer = 1, /**< GPU particle simulation parameters. */
		};

		/** Type of allocation, determining which buffers are allocated. */
		enum PoolType : u8
		{
			RenderablePool, /**< Just per-object buffer. */
			DecalPool, /**< Per-object buffer + decal parameter buffer. */
			GpuParticlesPool /**< Per-object buffer + GPU particle parameters. */
		};

		/** Configuration for a single buffer in the pool. */
		struct BufferConfiguration
		{
			BufferType Type; /**< Buffer type enum. */
			String UniformBufferParameterName; /**< Name for parameter set binding (e.g., "PerObject"). */
			u32 BufferSize; /**< Size of each suballocation in bytes. */
			GpuBufferFlag Flags; /**< Buffer creation flags. */
		};

		/** Configuration for registering a pool type. Each pool can allocate one or multiple buffers. */
		struct PoolConfiguration
		{
			PoolType Type;
			u32 EntriesPerBuffer; /**< Shared across all buffers in this type. */
			TInlineArray<BufferConfiguration, 4> Buffers; /**< Buffer configurations. */
			SPtr<GpuPipelineParameterSetLayout> Layout; /**< Parameter set layout for this type. */
		};

		/** Tracks a single allocation. Used for releasing the allocation once no longer needed. */
		struct AllocationHandle
		{
			/** Checks if this handle is valid. */
			bool IsValid() const { return Index != ~0u; }

		private:
			friend class UniformBufferPools;

			u32 Index = ~0u;
			PoolType Type = RenderablePool;
		};

		/** Result of a uniform buffer allocation. */
		struct AllocationResult
		{
			AllocationHandle Handle;
			SPtr<GpuParameterSet> ParameterSet;
			TInlineArray<GpuBufferSuballocation, 4> Suballocations; /**< Indexed by BufferType. */

			/** Checks if this result is valid. */
			bool IsValid() const { return Handle.IsValid(); }

			/** Gets suballocation by buffer type. */
			const GpuBufferSuballocation& GetSuballocation(BufferType type) const
			{
				return Suballocations[(u32)type];
			}
		};

		UniformBufferPools() = default;
		~UniformBufferPools() = default;

		/**
		 * Registers a type configuration. Must be called before Initialize().
		 *
		 * @param config	Configuration for this allocation type.
		 */
		void RegisterType(const PoolConfiguration& config);

		/**
		 * Initializes the manager. Must be called after all types are registered.
		 *
		 * @param device	GPU device for buffer creation.
		 */
		void Initialize(GpuDevice& device);

		/**
		 * Allocates uniform buffer(s) for a renderable.
		 *
		 * @param type		Type indicating which buffers to allocate.
		 * @return			Result containing handle, parameter set, and suballocations.
		 */
		AllocationResult Allocate(PoolType type = RenderablePool);

		/**
		 * Releases a uniform buffer allocation.
		 *
		 * @param handle	Handle from a previous Allocate() call.
		 */
		void Release(AllocationHandle handle);

		/**
		 * Updates per-object buffer using transform data stored in the render state.
		 *
		 * @param renderState	Render state whose per-object buffer should be updated.
		 * @param commandBuffer	Command buffer to queue the copy on. If null, uses the transfer command buffer.
		 */
		void UpdatePerObjectBuffer(const RenderState& renderState, const SPtr<GpuCommandBuffer>& commandBuffer = nullptr);

		/**
		 * Updates decal parameter buffer using data from a decal render state and proxy.
		 *
		 * @param decal			Decal render state whose decal param buffer should be updated.
		 * @param proxy			DecalProxy providing transform and property data.
		 * @param commandBuffer	Command buffer to queue the copy on. If null, uses the transfer command buffer.
		 */
		void UpdateDecalParamBuffer(const DecalRenderState& decal, const DecalProxy& proxy, const SPtr<GpuCommandBuffer>& commandBuffer = nullptr);

		/**
		 * Updates GPU particle parameter buffer using data from particles render state.
		 *
		 * @param particles		Particle render state whose GPU particle param buffer should be updated.
		 * @param commandBuffer	Command buffer to queue the copy on. If null, uses the transfer command buffer.
		 */
		void UpdateGpuParticlesParamBuffer(const ParticleRenderState& particles, const SPtr<GpuCommandBuffer>& commandBuffer = nullptr);

		/**
		 * Advances the staging pool frame counters. Call at end of each render frame.
		 */
		void AdvanceFrame();

		/**
		 * Releases all GPU resources held by this object.
		 *
		 * All allocations must be released before calling this method. Call before destroying the GPU device.
		 */
		void Destroy();

	private:
		static constexpr u32 kStagingEntriesPerBuffer = 256;

		/** Key for parameter set lookup - inline array of buffer pointers. */
		struct BufferKey
		{
			PoolType Type;
			TInlineArray<GpuBuffer*, 4> Buffers;  /**< Indexed by BufferType. */

			bool operator==(const BufferKey& other) const
			{
				if (Type != other.Type || Buffers.Size() != other.Buffers.Size())
					return false;

				for (u32 bufferIndex = 0; bufferIndex < Buffers.Size(); ++bufferIndex)
				{
					if (Buffers[bufferIndex] != other.Buffers[bufferIndex])
						return false;
				}

				return true;
			}
		};

		/** Hash functor for BufferKey. */
		struct BufferKeyHash
		{
			size_t operator()(const BufferKey& key) const
			{
				size_t hash = 0;
				B3DCombineHash(hash, (u32)key.Type);
				for (GpuBuffer* buffer : key.Buffers)
					B3DCombineHash(hash, buffer);

				return hash;
			}
		};

		/** Tracks GpuParameterSets shared by objects using the same underlying buffer combination. */
		struct BufferParameterSetEntry
		{
			SPtr<GpuParameterSet> ParameterSet;
			u32 RefCount = 0;
		};

		/** Internal entry tracking suballocations for a single allocation. */
		struct AllocationEntry
		{
			TInlineArray<GpuBufferSuballocation, 4> Suballocations; /**< Indexed by BufferType. */
			u32 NextFreeIndex = ~0u;
			bool IsAllocated = false;
		};

		/** A group of pools for one allocation type with independent tracking. */
		struct PoolGroup
		{
			PoolType Type;
			TInlineArray<BufferType, 4> PoolBufferTypes;
			TInlineArray<GpuBufferPool, 4> Pools;        /**< Pools indexed by BufferType. */
			TInlineArray<String, 4> UniformBufferNames;  /**< Uniform buffer names for parameter set binding. */
			u32 EntriesPerBuffer = 0;
			Vector<AllocationEntry> Entries;             /**< Per-type allocation entries. */
			u32 FreeListHead = kInvalidIndex;
			SPtr<GpuPipelineParameterSetLayout> ParameterSetLayout;
			UnorderedMap<BufferKey, BufferParameterSetEntry, BufferKeyHash> ParameterSetsByBuffer;

			static constexpr u32 kInvalidIndex = ~0u;
		};

		/**
		 * Gets or creates a shared GpuParameterSet for the given buffer combination.
		 *
		 * @param group		Pool group to use.
		 * @param entry		Allocation entry with suballocations.
		 * @return			Shared GpuParameterSet for set #1.
		 */
		SPtr<GpuParameterSet> GetOrCreateParameterSet(PoolGroup& group, const AllocationEntry& entry);

		/**
		 * Decrements ref count for a buffer combination's shared parameter set, removing if zero.
		 *
		 * @param group		Pool group to use.
		 * @param entry		Allocation entry with suballocations.
		 */
		void ReleaseParameterSet(PoolGroup& group, const AllocationEntry& entry);

		/**
		 * Builds a BufferKey from an allocation entry.
		 *
		 * @param type		Allocation type.
		 * @param entry		Allocation entry with suballocations.
		 * @return			BufferKey for parameter set lookup.
		 */
		BufferKey BuildBufferKey(PoolType type, const AllocationEntry& entry) const;

		TInlineArray<PoolGroup, 4> mPoolGroups;
		Vector<PoolConfiguration> mPendingConfigurations;
		TransientGpuBufferPool mPerObjectStagingPool;
		TransientGpuBufferPool mDecalStagingPool;
		TransientGpuBufferPool mGpuParticlesStagingPool;
		GpuDevice* mDevice = nullptr;
		bool mInitialized = false;
	};

	/** @} */
}
