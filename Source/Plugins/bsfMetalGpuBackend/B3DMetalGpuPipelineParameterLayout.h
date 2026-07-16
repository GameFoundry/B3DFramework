//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "GpuBackend/B3DGpuPipelineParameterLayout.h"

namespace b3d
{
	namespace render
	{
		class MetalGpuDevice;

		/** @addtogroup MetalGpuBackend
		 *  @{
		 */

		/**
		 * Describes a single binding within a Metal argument buffer in C++-visible form. Used by
		 * @c MetalGpuParameters to look up which argument-buffer slot, resource usage, and stage-mask
		 * should be applied when making resources resident on a command encoder.
		 */
		struct MetalArgumentBufferBinding
		{
			/** Engine slot index; what BSL/B3D callers pass to @c SetUniformBuffer / @c SetSampledTexture etc. */
			u32 Slot = 0;
			/** Dense logical index used for CPU-side dirty tracking. */
			u32 ArgIndex = 0;
			/** Byte offset of the first resource handle in the common Tier-2 argument-buffer layout. */
			u32 ByteOffset = 0;
			/** Byte distance between array elements in the common argument-buffer layout. */
			u32 ByteStride = 0;
			/** Engine parameter type: distinguishes uniform/storage buffer vs texture vs sampler. */
			GpuParameterType Type = GpuParameterType::Unknown;
			/** Metal object type (GPOT_*) for the binding; drives read/write usage flags. */
			GpuParameterObjectType ObjectType = GPOT_UNKNOWN;
			/** Array length of this slot; 1 for scalar bindings. */
			u32 ArraySize = 1;
			/** First element in the parameter set's dense resolved-resource cache. */
			u32 FirstResourceIndex = 0;
			/** First dense dynamic-offset index, or @c ~0u for non-dynamic bindings. */
			u32 DynamicOffsetIndex = ~0u;
			/** Mask of @c GpuProgramStageBit values indicating which shader stages reference this binding. */
			u32 StageMask = 0;
		};

		/** Holds the common, function-independent Tier-2 argument-buffer layout for one parameter set. */
		class MetalGpuPipelineParameterSetLayout : public GpuPipelineParameterSetLayout
		{
		public:
			MetalGpuPipelineParameterSetLayout(const GpuProgramParameterDescription& parameterDescription,
				const TShared<GpuResourceTableLayout>& resourceTableLayout, u32 tableIndex);
			~MetalGpuPipelineParameterSetLayout() override = default;

			/** Total size (in bytes) of the common argument buffer for this set. */
			u64 GetArgumentBufferSize() const { return mArgumentBufferSize; }

			/** Required alignment (in bytes) for the argument buffer's base offset. */
			u32 GetArgumentBufferAlignment() const { return mArgumentBufferAlignment; }

			/** Returns the full list of bindings this set exposes, in the order they were registered. */
			const TArray<MetalArgumentBufferBinding>& GetBindings() const { return mBindings; }

			/**
			 * B2: bindings pre-grouped by @c (MTLResourceUsage, MTLRenderStages) for the render path
			 * and by @c MTLResourceUsage alone for the compute path.
			 *
			 * One bucket collects every binding that shares usage + stage mask so
			 * @c useResources:count:usage:stages: (render) or @c useResources:count:usage: (compute)
			 * can mark the whole group resident in a single Metal call at draw/dispatch time, instead
			 * of the N @c useResource: calls the phase-1 path paid. @c Usage is precomputed from the
			 * binding's storage-writability; @c RenderStages is zero on compute buckets and unused.
			 *
			 * Cardinality in practice is small (@<= 4 render buckets, @<= 2 compute buckets) because
			 * the axes are tiny: usage is Read or ReadWrite, and shader stages are Vertex, Fragment,
			 * or Vertex|Fragment. Samplers are filtered out at bucket-build time.
			 */
			struct ArgumentBindingBucket
			{
#ifdef __OBJC__
				MTLResourceUsage Usage = MTLResourceUsageRead;
				MTLRenderStages RenderStages = (MTLRenderStages)0;
#else
				// Raw integer storage when the header is included from plain C++ TUs that cannot see
				// the Metal types. The @c .mm translation unit casts these to their Metal types.
				u64 Usage = 0;
				u64 RenderStages = 0;
#endif
				/** Dense resolved-resource indices in this bucket. Samplers are pre-filtered out. */
				TArray<u32> ResourceIndices;
			};

			/** Render-path buckets — see @c ArgumentBindingBucket. */
			const TArray<ArgumentBindingBucket>& GetRenderBuckets() const { return mRenderBuckets; }

			/** Compute-path buckets — see @c ArgumentBindingBucket. */
			const TArray<ArgumentBindingBucket>& GetComputeBuckets() const { return mComputeBuckets; }

			/**
			 * Returns the union of @c GpuProgramStageBit values referencing any binding in this set.
			 *
			 * Precomputed at construction so @c MetalGpuCommandBuffer does not need to re-walk every
			 * binding on each @c SetGpuParameterSet to decide which shader stages should receive the
			 * argument buffer. Consumed by @c AttachArgumentBufferToRenderEncoder /
			 * @c AttachArgumentBufferToComputeEncoder at bind time.
			 */
			u32 GetCombinedStageMask() const { return mCombinedStageMask; }

			/**
			 * Resolves an engine @c (type, slot) pair to its argument-buffer index within this set. Returns
			 * @c ~0u if no binding of that type exists at @p slot. Metal argument buffers use a single flat
			 * index space per set — this function is the authoritative mapping that both @c SetX calls and
			 * the SPIRV-Cross-emitted MSL agree on.
			 */
			u32 GetArgumentBufferIndex(GpuParameterType type, u32 slot, u32 arrayIndex = 0) const;

			/** Resolves a binding element to its byte offset within the common argument-buffer block. */
			u64 GetArgumentBufferByteOffset(GpuParameterType type, u32 slot, u32 arrayIndex = 0) const;

			/** Resolves a binding element to its dense resource-cache index, or @c ~0u if invalid. */
			u32 GetResourceIndex(GpuParameterType type, u32 slot, u32 arrayIndex = 0) const;

			/** Resolves a dense dynamic-offset index to its buffer binding. */
			bool GetDynamicOffsetBinding(u32 dynamicOffsetIndex, GpuParameterType& type, u32& slot, u32& arrayIndex) const;

		private:
			TArray<MetalArgumentBufferBinding> mBindings;
			u64 mArgumentBufferSize = 0;
			u32 mArgumentBufferAlignment = 16;

			// Union of stage masks across every binding in mBindings. Computed once after mBindings is
			// finalized so command-buffer bind paths can read the stage subset the set touches without
			// re-walking the bindings. See GetCombinedStageMask.
			u32 mCombinedStageMask = 0;

			// B2: precomputed bucket lists. Finalized in the ctor once @c mBindings is complete so
			// the command-buffer bind path reads a ready-made grouping at draw / dispatch time.
			TArray<ArgumentBindingBucket> mRenderBuckets;
			TArray<ArgumentBindingBucket> mComputeBuckets;
		};

		/** Holds meta-data about a set of GPU parameters used by a single pipeline state. */
		class MetalGpuPipelineParameterLayout : public GpuPipelineParameterLayout
		{
		public:
			MetalGpuPipelineParameterLayout(MetalGpuDevice& gpuDevice, const GpuPipelineParameterLayoutCreateInformation& createInformation);
			~MetalGpuPipelineParameterLayout() = default;
		};

		/** @} */
	} // namespace render
} // namespace b3d
