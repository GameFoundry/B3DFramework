//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/Allocators/B3DGpuAliasingAllocator.h"
#include "GpuBackend/B3DGpuSubmissionTimeline.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "Image/B3DTexture.h"

namespace b3d
{
	class GpuDevice;
	class GpuTransientResourceAllocator;

	/** @addtogroup GpuBackend
	 *  @{
	 */

	/** If disabled, transient resources are created in persistent memory, and never share memory with other resources. */
	extern B3D_EXPORT TConfigVariable<bool> gGpuTransientResources;

	/** Maximum number of textures each transient resource allocator caches. */
	extern B3D_EXPORT TConfigVariable<u32> gGpuTransientMaxCachedTextures;

	/** Maximum number of buffers each transient resource allocator caches. */
	extern B3D_EXPORT TConfigVariable<u32> gGpuTransientMaxCachedBuffers;

	/** Transient texture, and the barrier information its first use requires. */
	struct GpuTransientTexture
	{
		/** Allocated texture. Null if the allocation failed. */
		TShared<render::Texture> Texture;

		/**
		 * Earlier uses of the texture's memory that the texture's first use must be ordered after. Must be issued as
		 * GpuBarrier::AliasAcquire, before any other use of the texture. Remains valid until the end of the scope.
		 */
		render::GpuAliasAcquire Acquire;
	};

	/** Transient buffer, and the barrier information its first use requires. */
	struct GpuTransientBuffer
	{
		/** Allocated buffer. Null if the allocation failed. */
		TShared<render::GpuBuffer> Buffer;

		/**
		 * Earlier uses of the buffer's memory that the buffer's first use must be ordered after. Must be issued as
		 * GpuBarrier::AliasAcquire, before any other use of the buffer. Remains valid until the end of the scope.
		 */
		render::GpuAliasAcquire Acquire;
	};

	/** Memory use and resource counts of a GpuTransientResourceAllocator. */
	struct GpuTransientStatistics
	{
		GpuTransientScopeStatistics LastScope; /**< Memory use of the last ended scope. */
		u32 AllocationCount = 0; /**< Number of resources allocated in the last ended scope. */
		u32 CacheHits = 0; /**< Number of allocations in the last ended scope that reused a cached resource. */
		u32 CacheMisses = 0; /**< Number of allocations in the last ended scope that created and cached a new resource. */
		u32 CachedTextures = 0; /**< Number of textures currently cached. */
		u32 CachedBuffers = 0; /**< Number of buffers currently cached. */
	};

	/**
	 * Allocates transient resources within a scope opened with GpuTransientResourceAllocator::BeginScope().
	 *
	 * All allocations and releases of a scope must happen before any of the scope's GPU work is recorded, in an order
	 * consistent with the execution order on each queue. Every allocated resource must be released before the scope ends. A
	 * resource is valid from its allocation until its release, and its memory may be shared with resources allocated before
	 * or after that.
	 *
	 * @note	Same threading rules as the allocator.
	 */
	class B3D_EXPORT GpuTransientScope
	{
	public:
		/**
		 * Allocates a texture that is only accessed by the GPU, whose first use is in submission @p firstSubmission of the
		 * scope's timeline. The first use must not be on a transfer queue.
		 */
		GpuTransientTexture AllocateTexture(const TextureCreateInformation& createInformation, u32 firstSubmission);

		/** Allocates a buffer that is only accessed by the GPU, whose first use is in submission @p firstSubmission of the scope's timeline. */
		GpuTransientBuffer AllocateBuffer(const GpuBufferCreateInformation& createInformation, u32 firstSubmission);

		/**
		 * Releases a texture allocated in this scope. Its memory may be reused by later allocations of the scope, once their
		 * first use is ordered after @p lastUses.
		 *
		 * @param	texture		Texture to release.
		 * @param	lastUses	Last accesses of the texture, with one entry per queue that used it. Empty if the texture was never used.
		 */
		void Release(const render::Texture& texture, TArrayView<const GpuTransientLastUse> lastUses);

		/** @copydoc Release(const render::Texture&, TArrayView<const GpuTransientLastUse>) */
		void Release(const render::GpuBuffer& buffer, TArrayView<const GpuTransientLastUse> lastUses);

	private:
		friend class GpuTransientResourceAllocator;

		GpuTransientScope(GpuTransientResourceAllocator& allocator)
			: mAllocator(allocator)
		{ }

		GpuTransientResourceAllocator& mAllocator;
	};

	/**
	 * Allocates textures and buffers whose memory is shared with other resources with disjoint lifetimes. Resources are
	 * allocated in scopes, one per frame graph execution, each of which describes its GPU work with a GpuSubmissionTimeline.
	 * Memory is reused once the timeline orders the new resource's first use after the last uses of the earlier resources
	 * on the memory, on other queues. Last uses on the queue of the first use are returned as an alias acquire, which the
	 * first use must issue as a barrier.
	 *
	 * Memory comes from the device's transient heap pools, which every allocator shares. Each scope starts with all of the
	 * pools' memory free, so scopes of different allocators may be open at the same time, and share memory with each other.
	 * The system that schedules scopes must guarantee that no two scopes execute on the GPU at the same time, that the work of
	 * a scope is ordered after the work of every earlier scope, and that the work of a scope is submitted within the frame
	 * the scope ended in.
	 *
	 * Resources are cached by their description and memory location, so a scope that allocates the same resources in the
	 * same order as an earlier scope reuses the earlier resources. A cached resource's state from an earlier allocation is
	 * discarded by the alias acquire.
	 *
	 * @note	Not thread safe. Each scope's GPU work must be submitted in	timeline order. Two scopes must not record into 
	 *			the same command buffer.
	 */
	class B3D_EXPORT GpuTransientResourceAllocator
	{
	public:
		explicit GpuTransientResourceAllocator(GpuDevice& device);

		/** Releases all cached resources. The GPU must have finished using them. */
		~GpuTransientResourceAllocator();

		GpuTransientResourceAllocator(const GpuTransientResourceAllocator&) = delete;
		GpuTransientResourceAllocator& operator=(const GpuTransientResourceAllocator&) = delete;

		/**
		 * Opens a scope in which transient resources may be allocated, and releases cached resources that went unused for
		 * gGpuTransientIdleFrames frames. @p timeline describes the GPU work of the scope, and must remain valid and unchanged
		 * until EndScope(). Only one scope of this allocator may be open at a time.
		 */
		GpuTransientScope& BeginScope(const GpuSubmissionTimeline& timeline);

		/**
		 * Closes the scope opened with BeginScope(), and releases the least recently used cached resources beyond the cache
		 * size. Must be called after all of the scope's GPU work was recorded.
		 */
		void EndScope();

		/** Returns memory use and resource counts of the allocator. */
		GpuTransientStatistics GetStatistics() const;

	private:
		friend class GpuTransientScope;

		/** Transient resource, together with the memory it is placed at. */
		struct Resource
		{
			TShared<render::Texture> Texture; /**< Set for textures. */
			TShared<render::GpuBuffer> Buffer; /**< Set for buffers. */

			TextureInformation TextureDescription; /**< Description of the texture. Only valid for textures. */
			GpuBufferInformation BufferDescription; /**< Description of the buffer. Only valid for buffers. */
			GpuMemoryRequirements MemoryRequirements;

			/** Pool the memory is placed in, or null if the resource is in persistent memory. Resources in persistent memory are never cached. */
			IGpuTransientHeapPool* Pool = nullptr;
			GpuAllocation Allocation;

			u32 AllocationScope = 0; /**< Sequence number of the scope that allocated the resource last. */
			u64 AllocationFrame = 0; /**< Device frame index of the resource's last allocation. */

#if B3D_BUILD_TYPE_DEVELOPMENT
			TInlineArray<IGpuResource*, 4> Predecessors; /**< Earlier resources on the memory, as of the last allocation. */
			u32 AliasAcquireCount = 0; /**< Alias acquire count of the resource as of the last allocation. */
			bool IsReleasedWithUses = false; /**< True if the resource was released in the open scope, and used before that. */
#endif
		};

		/**
		 * Places memory for a resource with @p memoryRequirements and creates it with @p fnCreate, or reuses a cached resource
		 * that @p fnIsSame accepts. Falls back to persistent memory if the memory type cannot be aliased. Returns null if the
		 * resource could not be created.
		 */
		template<class CreateFunction, class IsSameFunction>
		Resource* AllocateResource(TArray<Resource*>& cachedResources, const GpuMemoryRequirements& memoryRequirements, u32 firstSubmission,
			render::GpuAliasAcquire& outAcquire, CreateFunction&& fnCreate, IsSameFunction&& fnIsSame);

		/** Releases a resource allocated in the open scope. See GpuTransientScope::Release(). */
		void ReleaseResource(const void* resourceKey, TArrayView<const GpuTransientLastUse> lastUses);

		/** Destroys a cached resource, and releases its heap reference. */
		static void EvictResource(Resource* resource);

		/** Returns the resource tracking the GPU use of @p resource, or null if the backend tracks none. */
		static IGpuResource* GetGpuResource(const Resource& resource);

		/** Returns true if @p a and @p b describe the same native texture. */
		static bool IsSameTexture(const TextureInformation& a, const TextureInformation& b);

		/** Returns true if @p a and @p b describe the same native buffer. */
		static bool IsSameBuffer(const GpuBufferInformation& a, const GpuBufferInformation& b);

		GpuDevice& mDevice;
		GpuTransientScope mScope;
		GpuAliasingAllocator mAliasingAllocator;

		const GpuSubmissionTimeline* mTimeline = nullptr; /**< Timeline of the open scope, or null if no scope is open. */
		u32 mScopeIndex = 0; /**< Sequence number of the latest scope. */

		TArray<Resource*> mCachedTextures;
		TArray<Resource*> mCachedBuffers;
		UnorderedMap<const void*, Resource*> mAllocatedResources; /**< Resources allocated in the open scope, keyed by their render proxy. */

#if B3D_BUILD_TYPE_DEVELOPMENT
		TArray<Resource*> mScopeReleasedResources; /**< Resources released in the open scope. */
#endif

		u32 mAllocationCount = 0;
		u32 mCacheHits = 0;
		u32 mCacheMisses = 0;
		GpuTransientStatistics mLastScopeStatistics;
	};

	/** @} */
} // namespace b3d
