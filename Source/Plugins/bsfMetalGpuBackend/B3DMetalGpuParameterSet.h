//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "Image/B3DTexture.h"

namespace b3d
{
	namespace render
	{
		class MetalGpuDevice;
		class MetalGpuCommandBuffer;
		class MetalGpuPipelineParameterSetLayout;
		class MetalGpuParameterSetPool;

		/** @addtogroup MetalGpuBackend
		 *  @{
		 */

		/**
		 * Metal implementation of a GPU parameter set backed by an argument buffer.
		 *
		 * Each set owns a single @c MTLBuffer whose layout is described by the set's
		 * @c MetalGpuPipelineParameterSetLayout. Every @c SetX call writes the resource handle into the
		 * argument buffer through the layout's shared @c MTLArgumentEncoder. The parameter set also
		 * retains strong references to the currently-bound resources so the command buffer can mark them
		 * resident (via @c useResource:usage:stages:) each time the set is bound.
		 */
		class MetalGpuParameters : public GpuParameterSet
		{
		public:
			/**
			 * @param gpuDevice				Device that owns the MTLBuffer-backing storage.
			 * @param parameterSetLayout	Argument-buffer layout — describes the MTLArgumentEncoder shape.
			 * @param setIndex				Engine-side set index, matching the BSL @c [[buffer(set)]] slot.
			 * @param pool					Optional pool to sub-allocate the argument buffer from (B9).
			 *								When @c nullptr falls back to a direct @c newBufferWithLength:
			 *								allocation — matches pre-B9 behavior and keeps unit tests that
			 *								create parameter sets outside a pool working.
			 */
			MetalGpuParameters(MetalGpuDevice& gpuDevice, const TShared<GpuPipelineParameterSetLayout>& parameterSetLayout, u32 setIndex, MetalGpuParameterSetPool* pool = nullptr);
			~MetalGpuParameters() override;

			bool SetUniformBuffer(u32 slot, const TShared<GpuBuffer>& uniformBuffer, u32 arrayIndex = 0, u32 offset = 0) override;
			bool SetSampledTexture(u32 slot, const TShared<Texture>& texture, const TextureSurface& surface = TextureSurface::kComplete, u32 arrayIndex = 0) override;
			bool SetStorageTexture(u32 slot, const TShared<Texture>& texture, const TextureSurface& surface, u32 arrayIndex = 0) override;
			bool SetStorageBuffer(u32 slot, const TShared<GpuBuffer>& buffer, u32 arrayIndex = 0, GpuBufferViewInformation view = GpuBufferViewInformation()) override;
			bool SetSamplerState(u32 slot, const TShared<SamplerState>& sampler, u32 arrayIndex = 0) override;

			void Initialize() override;

			/** Record of a single uniform buffer binding. */
			struct UniformBufferBinding
			{
				u32 Slot = 0;
				u32 ArrayIndex = 0;
				u32 Offset = 0;
				TShared<GpuBuffer> Buffer;
			};

			/** Record of a single storage buffer binding. */
			struct StorageBufferBinding
			{
				u32 Slot = 0;
				u32 ArrayIndex = 0;
				TShared<GpuBuffer> Buffer;
				GpuBufferViewInformation View;
			};

			/** Record of a single texture binding (sampled or storage). */
			struct TextureBinding
			{
				u32 Slot = 0;
				u32 ArrayIndex = 0;
				TShared<Texture> Texture;
				TextureSurface Surface;
			};

			/** Record of a single sampler-state binding. */
			struct SamplerBinding
			{
				u32 Slot = 0;
				u32 ArrayIndex = 0;
				TShared<SamplerState> Sampler;
			};

			/** Returns the list of currently bound uniform buffers, in insertion order. */
			const Vector<UniformBufferBinding>& GetUniformBuffers() const { return mUniformBuffers; }

			/** Returns the list of currently bound storage buffers, in insertion order. */
			const Vector<StorageBufferBinding>& GetStorageBuffers() const { return mStorageBuffers; }

			/** Returns the list of currently bound sampled textures, in insertion order. */
			const Vector<TextureBinding>& GetSampledTextures() const { return mSampledTextures; }

			/** Returns the list of currently bound storage textures, in insertion order. */
			const Vector<TextureBinding>& GetStorageTextures() const { return mStorageTextures; }

			/** Returns the list of currently bound sampler states, in insertion order. */
			const Vector<SamplerBinding>& GetSamplers() const { return mSamplers; }

			/** Sets the dynamic offset for the uniform buffer bound at the given slot. */
			bool SetDynamicOffset(u32 slot, u32 offset);

			/**
			 * Flushes any pending @c SetX / @c SetDynamicOffset writes into the argument buffer via the
			 * per-instance @c MTLArgumentEncoder. Called by @c MetalGpuCommandBuffer once per draw /
			 * dispatch so successive @c Set* calls for the same slot collapse to a single encoder write
			 * and identical re-binds across frames become no-ops. Short-circuits when no slot is dirty.
			 */
			void CommitPendingBindings();

			/**
			 * Returns @c true when at least one @c Set* / @c SetDynamicOffset call since the last
			 * @c CommitPendingBindings drained mutated a slot's (resource, array index, offset, size)
			 * tuple and the argument encoder therefore has pending writes. Cheap — used by the bind path
			 * to skip redundant @c CommitPendingBindings calls inside a draw command.
			 */
			bool HasPendingBindings() const;

			/**
			 * B3: monotonic counter bumped inside @c mSetMutex whenever a @c Set* call genuinely
			 * changed a slot's (resource, array index, offset, size) tuple. Command-buffer
			 * @c EmitResidencyFor{Render,Compute}Encoder reads this with the bound parameter set's
			 * raw identity and the *currently open encoder* to decide whether to re-emit
			 * @c useResources: calls at all. The counter is not bumped on @c SetDynamicOffset: dynamic
			 * offsets only change the encoded pointer inside the argument buffer, not the set of
			 * resources the driver needs to keep resident.
			 */
			u64 GetGeneration() const;

#ifdef __OBJC__
			/** Returns the argument buffer backing this set. May be nil if Initialize() has not been called. */
			id<MTLBuffer> GetArgumentBuffer() const;

			/**
			 * Returns the byte offset into @c GetArgumentBuffer() at which this set's slice begins.
			 * Non-zero only when the set was sub-allocated out of a pool block (B9). All encoder-side
			 * @c setVertexBuffer:offset: / @c setFragmentBuffer:offset: / @c setBuffer:offset: calls on
			 * the argument buffer must pass this value — passing zero overlaps with a neighbouring
			 * set's slice and silently corrupts bindings.
			 */
			u64 GetArgumentBufferOffset() const;

			/**
			 * Returns the cached Metal resource for the binding at @p argIndex (as assigned by the layout),
			 * or @c nil when the binding has no resource attached or @p argIndex is out of range. Maintained
			 * by @c CommitPendingBindings so command-buffer @c useResource: emission avoids re-resolving
			 * the underlying @c id<MTLResource> per bind.
			 */
			id<MTLResource> GetCachedResourceForArgIndex(u32 argIndex) const;
#endif

			/** Returns the typed layout used to build the argument buffer. */
			const MetalGpuPipelineParameterSetLayout* GetMetalLayout() const { return mMetalLayout; }

		private:
			struct Impl;

			/** Re-binds the shared argument encoder to this set's argument buffer prior to a write. */
			void BindEncoder() const;

			MetalGpuDevice& mGpuDevice;
			UPtr<Impl> mImpl;
			const MetalGpuPipelineParameterSetLayout* mMetalLayout = nullptr;

			// Optional pool that sub-allocated the argument buffer for this set. Non-owning pointer —
			// the pool outlives every set it created. nullptr when the parameter set was constructed
			// outside a pool, in which case Initialize falls back to a direct newBufferWithLength: alloc.
			MetalGpuParameterSetPool* mPool = nullptr;

			Vector<UniformBufferBinding> mUniformBuffers;
			Vector<StorageBufferBinding> mStorageBuffers;
			Vector<TextureBinding> mSampledTextures;
			Vector<TextureBinding> mStorageTextures;
			Vector<SamplerBinding> mSamplers;

			// Argument-buffer indices whose backing resource changed since the last CommitPendingBindings.
			// Set* / SetDynamicOffset only mutate the CPU-side Vector<>s and insert the resolved argIndex
			// here; the actual encoder writes happen once per commit, at which point the set is drained.
			// This makes a Set*-on-the-same-slot-twice pattern (common across frames) collapse to one
			// write, and makes a bind with identical bindings a no-op.
			UnorderedSet<u32> mDirtyArgumentSlots;

			// B4: per-slot snapshot of the last committed (resource pointer, Metal handle, array index,
			// offset/size) tuple. Set* compares its incoming tuple against this snapshot and only
			// inserts the argIndex into mDirtyArgumentSlots when the tuple genuinely differs, so a
			// "set-and-forget" binding pattern (common in material-driven rendering) stops paying
			// argument-encoder write cost every bind. Mirrors the Vulkan backend's descriptor-write
			// short-circuit.
			//
			// A'5: the snapshot stores *both* the engine-side TShared target address *and* the underlying
			// Metal handle (@c id<MTLResource> / @c id<MTLSamplerState>) and invalidates whenever
			// either differs. Engine-side pointer re-use is legal once a resource is dropped —
			// @c RecreateInternalTexture / @c RecreateInternalBuffer also swap the Metal handle under
			// an unchanged engine-side pointer — so comparing only the TShared address would let a stale
			// @c mResolvedResources[argIndex] survive past a recreate or a recycled allocation. The
			// Metal handle read is the same Obj-C field access the commit path already performs, so
			// the extra compare is effectively free.
			struct ArgumentSlotSnapshot
			{
				// Discriminator — matches @c GpuParameterType so a single map works for all 5 binding
				// kinds without tagging the slot key. Unknown means "never set".
				GpuParameterType Type = GpuParameterType::Unknown;
				// Untyped handle to the bound engine-side resource (GpuBuffer / Texture / SamplerState).
				// Compared via address; never dereferenced.
				const void* Resource = nullptr;
				// A'5: raw @c id<MTLResource> (or @c id<MTLSamplerState>) snapshot. Obj-C pointer stored
				// as @c void* so this header stays compilable in non-Obj-C++ plain-C++ TUs that
				// transitively include it; concrete @c __bridge casts live in the .mm. Compared via
				// address; never dereferenced here.
				void* MetalHandle = nullptr;
				u32 ArrayIndex = 0;
				// UniformBuffer: dynamic offset. StorageBuffer: view.FirstElement. Other: unused.
				u32 Offset = 0;
				// StorageBuffer: view.ElementCount. SampledTexture / StorageTexture: surface mip+face
				// packed. UniformBuffer / Sampler: unused. Granularity is ok to be coarse — goal is to
				// detect change, not reconstruct the value.
				u32 Size = 0;
			};
			UnorderedMap<u32, ArgumentSlotSnapshot> mSlotSnapshots;

			// B6: per-argIndex cache of the resolved id<MTLResource>. Populated lazily inside
			// CommitPendingBindings whenever a slot is flushed, so the command-buffer useResource:
			// emission pass reads from here rather than re-scanning the per-type Vector<>s for every
			// binding (the old O(n^2) resolve path). Values are type-erased as id<MTLResource> — the
			// downstream bind path only needs the resource handle, not its variant type.
			// Obj-C pointer stored as void* so the header compiles in non-ARC plain-C++ TUs that also
			// include it; concrete casts live in the .mm translation unit.
			Vector<void*> mResolvedResources;

			// Guards the Set* / SetDynamicOffset / CommitPendingBindings critical section. The base
			// GpuParameterSet contract declares the set thread-safe, so two sim-thread fibers may hit
			// SetX concurrently; without this lock the base-class array writes and the Metal-side
			// Vector<> + mDirtyArgumentSlots mutation would tear. Also held around the argument-encoder
			// writes in CommitPendingBindings because the MTLArgumentEncoder carries per-instance state
			// (the bound argument buffer + offset) that must not be reinterpreted mid-commit.
			mutable Mutex mSetMutex;

			// B3: generation counter. Bumped inside @c mSetMutex from every @c Set* path that genuinely
			// dirtied a slot (the B4 value-compare above is the gate). Not bumped on
			// @c SetDynamicOffset — dynamic offsets change the encoded pointer but not the resident
			// resource set. Starts at 1 so a freshly-default-constructed cache slot (value 0) never
			// matches. Only written under mSetMutex; only read via GetGeneration on the command-buffer
			// owner thread after the submitting caller has taken a natural happens-before through
			// Set* -> SetGpuParameterSet -> bind.
			u64 mGeneration = 1;
		};

		/** @} */
	} // namespace render
} // namespace b3d
