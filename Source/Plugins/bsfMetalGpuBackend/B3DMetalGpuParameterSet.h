//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "B3DMetalGpuBuffer.h"
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
		class MetalResourceTracker;
		class MetalBarrierHelper;

		/** @addtogroup MetalGpuBackend
		 *  @{
		 */

		/**
		 * Heap-placed argument buffer owned by a single MetalGpuParameters. Wrapped in a tracked resource so that its
		 * release, and the return of its span to the heap allocator, is deferred until every command buffer that encoded
		 * it has retired. 
		 */
		class MetalArgumentBuffer : public MetalResource
		{
		public:
			/**
			 * @param	owner		Manager responsible for the deferred release.
			 * @param	buffer		Placed buffer. Takes ownership of the +1 reference.
			 * @param	allocation	Allocator span backing @p buffer, or an invalid location for a direct device allocation.
			 */
			MetalArgumentBuffer(MetalResourceManager* owner, MetalBufferNativeHandle buffer, const GpuResourceLocation& allocation);
			~MetalArgumentBuffer();

#ifdef __OBJC__
			/** Returns the native buffer. */
			id<MTLBuffer> GetMetalBuffer() const { return mBuffer; }
#endif

		private:
			MetalBufferNativeHandle mBuffer = nullptr;
			GpuResourceLocation mAllocation;
		};

		/**
		 * Metal implementation of a GPU parameter set backed by an argument buffer.
		 *
		 * Each set owns one Tier-2 argument-buffer allocation shared by every shader stage. Bound resources are held by
		 * the base class; @c PrepareForBind resolves them to Metal handles and writes whichever argument-buffer entries
		 * changed, so the command buffer can then mark the resources resident (via @c useResources:usage:stages:).
		 *
		 * Uniform buffers declared with a dynamic offset are not written into the argument buffer; the command buffer
		 * binds them directly in the encoder's argument table (see @c GetBoundUniformBuffer) so per-draw offset changes
		 * never touch the argument buffer.
		 *
		 * @note	Not thread safe. Used only on the thread that owns the pool it was created from.
		 */
		class MetalGpuParameters : public GpuParameterSet
		{
		public:
			/**
			 * @param gpuDevice				Device that owns the MTLBuffer-backing storage.
			 * @param parameterSetLayout	Description and layout of the parameters within the buffer set.
			 * @param setIndex				Engine-side set index. Also the Metal buffer index the set's argument buffer binds to,
			 *								declared as @c [[buffer(setIndex)]] in the generated MSL.
			 * @param pool					Transient pool to sub-allocate the argument buffer from. When @c nullptr
			 *								the set allocates its own argument buffer through the device's heap allocator.
			 */
			MetalGpuParameters(MetalGpuDevice& gpuDevice, const TShared<GpuPipelineParameterSetLayout>& parameterSetLayout, u32 setIndex, MetalGpuParameterSetPool* pool = nullptr);
			~MetalGpuParameters() override;

			bool SetUniformBuffer(u32 slot, const TShared<GpuBuffer>& uniformBuffer, u32 arrayIndex = 0, u32 offset = 0) override;
			bool SetSampledTexture(u32 slot, const TShared<Texture>& texture, const TextureSurface& surface = TextureSurface::kComplete, u32 arrayIndex = 0) override;
			bool SetStorageTexture(u32 slot, const TShared<Texture>& texture, const TextureSurface& surface, u32 arrayIndex = 0) override;
			bool SetStorageBuffer(u32 slot, const TShared<GpuBuffer>& buffer, u32 arrayIndex = 0, GpuBufferViewInformation view = GpuBufferViewInformation()) override;
			bool SetSamplerState(u32 slot, const TShared<SamplerState>& sampler, u32 arrayIndex = 0) override;

			void Initialize() override;

			/**
			 * Returns the uniform buffer bound at @p slot together with the offset it was bound with, or null when the
			 * slot has no buffer.
			 */
			GpuBuffer* GetBoundUniformBuffer(u32 slot, u32& outOffset) const;

			/**
			 * Prepares the set for binding by writing the Metal handles of the bound resources into the argument buffer.
			 * Called by @c MetalGpuCommandBuffer once per draw / dispatch. Only entries whose value changed are written,
			 * so identical re-binds across frames are no-ops. When no @c Set* call happened since the last prepare, only
			 * checks whether any bound buffer or texture swapped its backing Metal resource.
			 *
			 * If the argument buffer is still bound to a recording or in-flight command buffer, the writes go to a fresh
			 * copy (copy-on-write) and the generation is bumped so the command buffer re-attaches it.
			 *
			 * @return	Current generation, which changes whenever the command buffer must re-attach the argument
			 *			buffer and re-emit residency.
			 */
			u64 PrepareForBind();

			/**
			 * Registers every resource bound in this set with a command buffer's resource tracker.
			 *
			 * @param	resourceTracker			Tracker of the command buffer the set is bound on.
			 * @param	barrierHelper			Receives synchronization required before accessing the resources.
			 * @param	dynamicOffsetOverrides	Command buffer's per-draw offset overrides for the set, indexed by
			 *									dynamic-offset index (@c ~0u for none), so uniform-buffer suballocations
			 *									are tracked at the offset the draw actually reads.
			 * @param	compute					True when tracking for a dispatch, false for a draw.
			 * @return							False if shader accesses overlap a writable attachment.
			 */
			bool TrackResources(MetalResourceTracker& resourceTracker, MetalBarrierHelper& barrierHelper, const TInlineArray<u32, 4>& dynamicOffsetOverrides, bool compute) const;

			/**
			 * Returns the tracked resource owning the argument buffer, or null when the buffer is a slice of a transient
			 * pool block (whose lifetime the pool's Reset contract covers instead). Command buffers that encode the
			 * argument buffer must track this resource so its release is deferred until they retire.
			 */
			MetalArgumentBuffer* GetArgumentBufferResource() const;

#ifdef __OBJC__
			/** Returns the argument buffer backing this set. May be nil if Initialize() has not been called. */
			id<MTLBuffer> GetArgumentBuffer() const;

			/**
			 * Returns the byte offset into @c GetArgumentBuffer() at which this set's slice begins.
			 * Non-zero only when the set was sub-allocated out of a pool block. All command-encoder
			 * @c setVertexBuffer:offset: / @c setFragmentBuffer:offset: / @c setBuffer:offset: calls on
			 * the argument buffer must pass this value — passing zero overlaps with a neighbouring
			 * set's slice and silently corrupts bindings.
			 */
			u64 GetArgumentBufferOffset() const;

			/**
			 * Returns the Metal resource resolved for the layout's dense @p resourceIndex, or @c nil when the binding has
			 * no resource or the index is out of range. Maintained by @c PrepareForBind so command-buffer
			 * @c useResource: emission avoids re-resolving the underlying @c id<MTLResource> per bind.
			 */
			id<MTLResource> GetCachedResource(u32 resourceIndex) const;
#endif

			/** Returns the typed layout used to build the argument buffer. */
			const MetalGpuPipelineParameterSetLayout* GetMetalLayout() const { return mMetalLayout; }

			/** Returns whether the argument buffer was successfully allocated. Always true when the layout requires no argument buffer. */
			bool IsArgumentBufferAllocated() const;

		private:
			struct Impl;

			/** State of a single argument-buffer element (one array entry of one binding). */
			struct ArgumentElement
			{
				/** Value last written to the argument buffer: a GPU address for buffers, a resource ID otherwise. */
				u64 Value = 0;

				/**
				 * Resource the element resolved to, for residency: the buffer, texture view or texture-buffer view.
				 * Null for samplers, which need no residency. Stored as @c void* so this header compiles in plain C++
				 * translation units; the casts live in the .mm.
				 */
				void* ResolvedResource = nullptr;

				/**
				 * Backing @c id<MTLBuffer> / @c id<MTLTexture> of the bound engine resource when it was last resolved.
				 * Buffers and textures can swap it on a discard write without any @c Set* call, so PrepareForBind
				 * compares against it to catch that. Compared, never dereferenced.
				 */
				void* NativeHandle = nullptr;
			};

			MetalGpuDevice& mGpuDevice;
			TUnique<Impl> mImpl;
			const MetalGpuPipelineParameterSetLayout* mMetalLayout = nullptr;

			MetalGpuParameterSetPool* mPool = nullptr;

			/** Per-element state, indexed by MetalArgumentBufferBinding::FirstResourceIndex + array index. */
			Vector<ArgumentElement> mElements;

			/**
			 * Set by every @c Set* call. PrepareForBind resolves every element while it is set, and otherwise only
			 * checks the elements' native handles.
			 */
			bool mBindingsDirty = true;

			/**
			 * Generation counter. Bumped whenever PrepareForBind changes a resolved resource, and when it moves the
			 * argument buffer to a fresh slice (copy-on-write), so the command buffer re-attaches the argument buffer
			 * and re-emits residency. Starts at 1 so a freshly-default-constructed cache slot (value 0) never matches.
			 */
			u64 mGeneration = 1;
		};

		/** @} */
	} // namespace render
} // namespace b3d
