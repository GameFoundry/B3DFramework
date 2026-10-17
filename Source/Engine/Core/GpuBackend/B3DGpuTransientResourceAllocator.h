//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/Allocators/B3DGpuAliasingAllocator.h"
#include "GpuBackend/B3DGpuSubmissionTimeline.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "Image/B3DTexture.h"
#include "Utility/B3DPool.h"

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
		 * Earlier uses of the texture's memory that the texture's first use must be ordered after. Must be issued before any other use
		 * of the texture, as GpuBarrier::AliasAcquire, or as an explicit barrier with GpuBarrierFlag::AliasAcquire whose source is
		 * GpuAliasAcquire::Source. Remains valid until the end of the scope.
		 */
		render::GpuAliasAcquire Acquire;

		/**
		 * Latest pass among the accesses of Acquire, as reported by GpuTransientLastUse::Pass when the earlier resources on the memory
		 * were released. When the alias acquire is issued as a split barrier, its release half can go right after this pass, and its
		 * acquire half before the first use. Empty if Acquire has no accesses.
		 */
		TOptional<u32> LatestSourcePass;
	};

	/** Transient buffer, and the barrier information its first use requires. */
	struct GpuTransientBuffer
	{
		/** Allocated buffer. Null if the allocation failed. */
		TShared<render::GpuBuffer> Buffer;

		/** Earlier uses of the buffer's memory that the buffer's first use must be ordered after. See GpuTransientTexture::Acquire. */
		render::GpuAliasAcquire Acquire;

		/** See GpuTransientTexture::LatestSourcePass. */
		TOptional<u32> LatestSourcePass;
	};

	/** Result of placing memory for a transient resource returned by GpuTransientResourceAllocator::GetOrCreateTexture() or GetOrCreateBuffer(). */
	struct GpuTransientPlacement
	{
		/** Earlier uses of the resource's memory that the resource's first use must be ordered after. See GpuTransientTexture::Acquire. */
		render::GpuAliasAcquire Acquire;

		/** See GpuTransientTexture::LatestSourcePass. */
		TOptional<u32> LatestSourcePass;

		/**
		 * True if the resource recreated its native resource at new memory. Views, render targets and other objects created from the
		 * native resource before the placement are no longer valid.
		 */
		bool Recreated = false;
	};

	/** Memory use and resource counts of a GpuTransientResourceAllocator. */
	struct GpuTransientStatistics
	{
		GpuTransientScopeStatistics LastScope; /**< Memory use of the last ended scope. */
		u32 PlacementCount = 0; /**< Number of resources placed in the last ended scope, allocated ones included. */
		u32 CacheHits = 0; /**< Number of placements in the last ended scope that kept a cached resource at its memory. */
		u32 CacheMisses = 0; /**< Number of placements in the last ended scope that cached a new resource. */
		u32 Relocations = 0; /**< Number of placements in the last ended scope that recreated a cached resource at other memory. */
		u32 CachedTextures = 0; /**< Number of textures currently cached. */
		u32 CachedBuffers = 0; /**< Number of buffers currently cached. */
	};

	/**
	 * Places transient resources within a scope opened with GpuTransientResourceAllocator::BeginScope(). See
	 * GpuTransientResourceAllocator for the terms.
	 *
	 * All placements and releases of a scope must happen before any of the scope's GPU work is recorded, in an order consistent
	 * with the execution order on each queue. Every placed resource must be released before the scope ends. A resource is valid
	 * from its placement until its release, and its memory may be shared with resources placed before or after that.
	 *
	 * @note	Same threading rules as the allocator.
	 */
	class B3D_EXPORT GpuTransientScope
	{
	public:
		/**
		 * Allocates a texture that is only accessed by the GPU: gets or creates it and places it in one call. Its first use is in
		 * submission @p firstSubmission of the scope's timeline, and must not be on a transfer queue.
		 */
		GpuTransientTexture AllocateTexture(const TextureCreateInformation& createInformation, u32 firstSubmission);

		/** Allocates a buffer that is only accessed by the GPU. Its first use is in submission @p firstSubmission of the scope's timeline. See AllocateTexture(). */
		GpuTransientBuffer AllocateBuffer(const GpuBufferCreateInformation& createInformation, u32 firstSubmission);

		/**
		 * Places memory for a texture returned by GpuTransientResourceAllocator::GetOrCreateTexture(), whose first use is in submission
		 * @p firstSubmission of the scope's timeline. The texture keeps its memory from an earlier scope if that memory is free,
		 * otherwise it is recreated at other memory. The first use must not be on a transfer queue.
		 */
		GpuTransientPlacement PlaceTexture(const render::Texture& texture, u32 firstSubmission);

		/** Places memory for a buffer returned by GpuTransientResourceAllocator::GetOrCreateBuffer(). See PlaceTexture(). */
		GpuTransientPlacement PlaceBuffer(const render::GpuBuffer& buffer, u32 firstSubmission);

		/**
		 * Releases a texture placed in this scope. Its memory may be reused by later placements of the scope, once
		 * their first use is ordered after @p lastUses.
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
	 * placed in scopes, one per frame graph execution, each of which describes its GPU work with a GpuSubmissionTimeline.
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
	 * Resources are cached by their description and memory location, so a scope that places the same resources in the
	 * same order as an earlier scope reuses the earlier resources. A cached resource's state from an earlier placement is
	 * discarded by the alias acquire.
	 *
	 * Terms:
	 *  - Get or create: GetOrCreateTexture() returns a cached texture, or creates one if none matches, before the texture's
	 *    first use is known. The texture has no memory for the scope yet. It is unplaced until it is placed, and accessing an
	 *    unplaced texture on the GPU is invalid.
	 *  - Place: GpuTransientScope::PlaceTexture() gives an unplaced texture memory for its first use in the scope.
	 *  - Allocate: GpuTransientScope::AllocateTexture() gets or creates a texture and places it in one call, once its first use
	 *    is known. It picks a cached texture whose memory is free, so it is not equivalent to GetOrCreateTexture() followed by
	 *    PlaceTexture().
	 *  - Release: GpuTransientScope::Release() ends the texture's lifetime in the scope, so later placements may reuse its memory.
	 *  - Held: a texture is held from the moment GetOrCreateTexture() or AllocateTexture() returns it until the end of the scope
	 *    it was placed in. GetOrCreateTexture() never returns a held texture, and eviction skips it.
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
		 * Returns a texture that is only accessed by the GPU, without memory. The texture must be placed with
		 * GpuTransientScope::PlaceTexture() before its first use, given back with Return(), or moved out of the allocator with
		 * Detach(). Returns the first cached texture with a matching description that is not held, so a scope that requests the
		 * same textures in the same order as an earlier scope gets the same textures. Otherwise creates a new, uninitialized
		 * texture. Returns null if the texture cannot be created.
		 */
		TShared<render::Texture> GetOrCreateTexture(const TextureCreateInformation& createInformation);

		/** Returns a buffer that is only accessed by the GPU, without memory. See GetOrCreateTexture(). */
		TShared<render::GpuBuffer> GetOrCreateBuffer(const GpuBufferCreateInformation& createInformation);

		/** Gives back a texture returned by GetOrCreateTexture() and never placed. */
		void Return(const render::Texture& texture);

		/** Gives back a buffer returned by GetOrCreateBuffer() and never placed. */
		void Return(const render::GpuBuffer& buffer);

		/**
		 * Removes a texture returned by GetOrCreateTexture() and never placed from the allocator. The caller owns the
		 * texture from then on. If the texture is initialized, it remains on transient memory, which the caller must move it off of
		 * with render::Texture::SetAllocation() before the allocator's next scope begins.
		 *
		 * A texture that GetOrCreateTexture() took from the cache has a native resource on transient memory, so moving it costs a recreation,
		 * and the next texture with its description must be created anew.
		 */
		void Detach(const render::Texture& texture);

		/** Removes a buffer returned by GetOrCreateBuffer() and never placed from the allocator. See Detach(const render::Texture&). */
		void Detach(const render::GpuBuffer& buffer);

		/**
		 * Opens a scope in which transient resources may be placed, and releases cached resources that went unused for
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

			u32 PlacementScope = 0; /**< Sequence number of the scope that placed the resource last. */
			u64 LastRequestFrame = 0; /**< Device frame index of the last time the resource was returned to a caller or placed. */

			/** True from the moment the resource is returned to a caller until the end of the scope it was placed in. See GpuTransientResourceAllocator. */
			bool IsHeld = false;

			TInlineArray<IGpuResource*, 4> Predecessors; /**< Earlier resources on the memory, as of the last placement. */
		};

		/**
		 * Places memory for a resource with @p memoryRequirements and creates it with @p fnCreate, or reuses a cached resource
		 * that @p fnIsSame accepts. Falls back to persistent memory if the memory type cannot be aliased. Returns null if the
		 * resource could not be created. Outputs the acquire of the resource's first use, and the latest pass among its accesses.
		 */
		template<class CreateFunction, class IsSameFunction>
		Resource* AllocateResource(TArray<Resource*>& cachedResources, const GpuMemoryRequirements& memoryRequirements, u32 firstSubmission,
			render::GpuAliasAcquire& outAcquire, TOptional<u32>& outLatestSourcePass, CreateFunction&& fnCreate, IsSameFunction&& fnIsSame);

		/**
		 * Returns the first cached resource that @p fnIsSame accepts and that is not held, or creates an uninitialized resource
		 * with @p fnCreate. Returns null if the resource could not be created. See GetOrCreateTexture().
		 */
		template<class CreateFunction, class IsSameFunction>
		Resource* GetOrCreateResource(TArray<Resource*>& cachedResources, const GpuMemoryRequirements& memoryRequirements, CreateFunction&& fnCreate, IsSameFunction&& fnIsSame);

		/** Places memory for a resource returned by GetOrCreateResource(). See GpuTransientScope::PlaceTexture(). */
		GpuTransientPlacement PlaceResource(TArray<Resource*>& cachedResources, const void* resourceKey, u32 firstSubmission);

		/** Records the memory a resource was placed at in the open scope, and completes @p acquire with development information. */
		void NotifyResourcePlaced(Resource& resource, const GpuAllocation& allocation, render::GpuAliasAcquire& acquire);

		/** Releases a resource placed in the open scope. See GpuTransientScope::Release(). */
		void ReleaseResource(const void* resourceKey, TArrayView<const GpuTransientLastUse> lastUses);

		/**
		 * Gives back a resource returned by GetOrCreateResource() and never placed. If @p detach is true, also removes it from the
		 * cache. See Return() and Detach().
		 */
		void ReturnResource(TArray<Resource*>& cachedResources, const void* resourceKey, bool detach);

		/** Finds the memory requirements of a cached resource that @p fnIsSame accepts. Returns false if there is none. */
		template<class IsSameFunction>
		static bool FindCachedMemoryRequirements(const TArray<Resource*>& cachedResources, IsSameFunction&& fnIsSame, GpuMemoryRequirements& outMemoryRequirements);

		/** Returns the render proxy of the resource, which identifies it to the scope. */
		static const void* GetResourceKey(const Resource& resource);

#if B3D_BUILD_TYPE_DEVELOPMENT
		/** Marks the resource as unplaced. See render::Texture::SetUnplaced(). */
		static void SetUnplaced(const Resource& resource, bool unplaced);
#endif

		/** Destroys a cached resource, and releases its heap reference. */
		void EvictResource(Resource* resource);

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

		TArray<Resource*> mCachedTextures; /**< In the order they were cached. */
		TArray<Resource*> mCachedBuffers; /**< In the order they were cached. */
		TPool<Resource> mResourcePool; /**< Storage of every resource the allocator tracks. */
		UnorderedMap<const void*, Resource*> mUnplacedResources; /**< Resources returned by GetOrCreateResource() and not yet placed, keyed by their render proxy. */
		UnorderedMap<const void*, Resource*> mPlacedResources; /**< Resources placed in the open scope and not yet released, keyed by their render proxy. */
		TArray<Resource*> mScopeHeldResources; /**< Cached resources placed in the open scope, held until it ends. */

		u32 mPlacementCount = 0;
		u32 mCacheHits = 0;
		u32 mCacheMisses = 0;
		u32 mRelocations = 0;
		GpuTransientStatistics mLastScopeStatistics;
	};

	/** @} */
} // namespace b3d
