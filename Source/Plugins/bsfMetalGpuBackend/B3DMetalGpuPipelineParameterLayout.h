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
			/** Byte offset of the first resource handle within the set's argument buffer. Every stage reads the same struct. */
			u32 ByteOffset = 0;
			/** Byte distance between array elements. */
			u32 ByteStride = 0;
			/** Engine parameter type: distinguishes uniform/storage buffer vs texture vs sampler. */
			GpuParameterType Type = GpuParameterType::Unknown;
			/** Metal object type (GPOT_*) for the binding; drives read/write usage flags. */
			GpuParameterObjectType ObjectType = GPOT_UNKNOWN;
			/** Array length of this slot; 1 for scalar bindings. */
			u32 ArraySize = 1;
			/** First element in the parameter set's dense resolved-resource cache. */
			u32 FirstResourceIndex = 0;
			/** Mask of @c GpuProgramStageBit values indicating which shader stages reference this binding. */
			u32 StageMask = 0;
		};

		/**
		 * Describes a uniform buffer that binds directly in the per-stage argument table with a dynamic offset, bypassing
		 * the parameter set's argument buffer. Changing its offset is a single encoder call per stage.
		 */
		struct MetalDynamicUniformBufferBinding
		{
			/** Engine set the binding belongs to. */
			u32 Set = 0;
			/** Engine slot index within the set. */
			u32 Slot = 0;
			/** Dynamic-offset index within the set's layout (see @c GpuPipelineParameterSetLayout::GetDynamicOffsetIndex). */
			u32 DynamicOffsetIndex = ~0u;
			/**
			 * Argument-table buffer index, shared by every stage that reads the buffer. @c ~0u for a layout created
			 * without shader reflection, which describes parameter-set storage only and never drives a bind.
			 */
			u32 BufferIndex = ~0u;
			/** Mask of @c GpuProgramStageBit values indicating which shader stages reference this binding. */
			u32 StageMask = 0;
		};

		/**
		 * Holds the Tier-2 argument-buffer layout for one parameter set, plus the argument-table indices of the set's
		 * dynamic-offset uniform buffers. Every stage of a pipeline is generated from the same declared resources, so
		 * one argument-buffer struct serves all of them: a stage's reflected table merely omits the members that stage
		 * never reads.
		 */
		class MetalGpuPipelineParameterSetLayout : public GpuPipelineParameterSetLayout
		{
		public:
			/**
			 * Reflected resource-table layout of one shader stage and the index of the descriptor table backing this set's
			 * argument buffer within it, @c ~0u when the stage reads the set only through argument-table uniform buffers.
			 */
			struct StageReflectedTable
			{
				TShared<GpuResourceTableLayout> Layout;
				u32 TableIndex = ~0u;
			};

			/**
			 * Creates the layout without shader reflection: bindings pack densely in registration order and dynamic-offset
			 * uniform buffers receive no argument-table index. @c MetalGpuPipelineParameterLayout applies reflection
			 * afterwards through RebuildWithStageTables; the reflected arguments only satisfy the device factory interface.
			 */
			MetalGpuPipelineParameterSetLayout(const GpuProgramParameterDescription& parameterDescription,
				const TShared<GpuResourceTableLayout>& resourceTableLayout, u32 tableIndex);
			~MetalGpuPipelineParameterSetLayout() override = default;

			/**
			 * Re-derives the argument-buffer offsets and argument-table indices from the reflected tables of every stage
			 * that references the set. Must run before the layout is published to any parameter set.
			 */
			void RebuildWithStageTables(TArrayView<const StageReflectedTable> stageTables) { Build(stageTables); }

			/** Total size (in bytes) of the common argument buffer for this set. */
			u64 GetArgumentBufferSize() const { return mArgumentBufferSize; }

			/** Required alignment (in bytes) for the argument buffer's base offset. */
			u32 GetArgumentBufferAlignment() const { return mArgumentBufferAlignment; }

			/** Returns the argument-buffer bindings this set exposes, in the order they were registered. Dynamic-offset uniform buffers are not included. */
			const TArray<MetalArgumentBufferBinding>& GetBindings() const { return mBindings; }

			/** Returns the uniform buffers that bind in the argument table with a dynamic offset, sorted by slot. */
			const TArray<MetalDynamicUniformBufferBinding>& GetDynamicUniformBufferBindings() const { return mDynamicUniformBufferBindings; }

			/** Finds the argument-table binding of the uniform buffer at @p slot, or null when the slot is not a dynamic-offset uniform buffer. */
			const MetalDynamicUniformBufferBinding* FindDynamicUniformBufferBinding(u32 slot) const;

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

			/** Finds the binding record for an engine @c (type, slot) pair, or null when absent. */
			const MetalArgumentBufferBinding* FindBinding(GpuParameterType type, u32 slot) const;

			/** Resolves a binding element to its dense resource-cache index, or @c ~0u if invalid. */
			u32 GetResourceIndex(GpuParameterType type, u32 slot, u32 arrayIndex = 0) const;

		private:
			/** Shared build path of the constructor and RebuildWithStageTables. An empty @p stageTables selects the dense packing. */
			void Build(TArrayView<const StageReflectedTable> stageTables);

			TArray<MetalArgumentBufferBinding> mBindings;
			TArray<MetalDynamicUniformBufferBinding> mDynamicUniformBufferBindings;
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

			/** Returns the shader stages that consume the standalone push-constant block. */
			u32 GetPushConstantStageMask() const { return mPushConstantStageMask; }

		private:
			u32 mPushConstantStageMask = 0;
		};

		/** @} */
	} // namespace render
} // namespace b3d
