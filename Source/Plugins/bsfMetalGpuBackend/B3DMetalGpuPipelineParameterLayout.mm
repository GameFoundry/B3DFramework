//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalGpuPipelineParameterLayout.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalBytecodeLayout.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"
#include "Utility/B3DCommonTypes.h"
#include <algorithm>

namespace b3d
{
	namespace render
	{
		struct MetalGpuPipelineParameterSetLayout::Impl
		{
			// Retained MTLArgumentDescriptor list. Kept so each MetalGpuParameters can spin up its own
			// MTLArgumentEncoder without re-walking the base-class parameter description. Empty when the
			// layout has no bindings.
			NSMutableArray<MTLArgumentDescriptor*>* Descriptors = nil;
		};

		namespace
		{
			/** Maps the engine's per-stage usage flags into a bitmask over @c GpuProgramStageBit values. */
			u32 BuildStageMask(const GpuProgramStageBits& usage)
			{
				u32 mask = 0;
				if (usage.IsSet(GpuProgramStageBit::Vertex))
					mask |= (u32)GpuProgramStageBit::Vertex;
				if (usage.IsSet(GpuProgramStageBit::Fragment))
					mask |= (u32)GpuProgramStageBit::Fragment;
				if (usage.IsSet(GpuProgramStageBit::Compute))
					mask |= (u32)GpuProgramStageBit::Compute;
				if (usage.IsSet(GpuProgramStageBit::Hull))
					mask |= (u32)GpuProgramStageBit::Hull;
				if (usage.IsSet(GpuProgramStageBit::Domain))
					mask |= (u32)GpuProgramStageBit::Domain;
				if (usage.IsSet(GpuProgramStageBit::Geometry))
					mask |= (u32)GpuProgramStageBit::Geometry;
				return mask;
			}

			/** Returns true if the given Metal object type represents a texture that may be written from a shader. */
			bool IsWritableTextureType(GpuParameterObjectType type)
			{
				switch (type)
				{
				case GPOT_RWTEXTURE1D:
				case GPOT_RWTEXTURE2D:
				case GPOT_RWTEXTURE3D:
				case GPOT_RWTEXTURE2DMS:
				case GPOT_RWTEXTURE1DARRAY:
				case GPOT_RWTEXTURE2DARRAY:
				case GPOT_RWTEXTURE2DMSARRAY:
					return true;
				default:
					return false;
				}
			}

			/** Returns true if the given buffer object type represents a buffer that may be written from a shader. */
			bool IsWritableBufferType(GpuParameterObjectType type)
			{
				switch (type)
				{
				case GPOT_RWBYTE_BUFFER:
				case GPOT_RWTYPED_BUFFER:
				case GPOT_RWSTRUCTURED_BUFFER:
				case GPOT_RWSTRUCTURED_BUFFER_WITH_COUNTER:
				case GPOT_RWAPPEND_BUFFER:
				case GPOT_RWCONSUME_BUFFER:
					return true;
				default:
					return false;
				}
			}

			/**
			 * B2: Metal resource-usage flags for a single argument-buffer binding. Writable storage
			 * resources get Read|Write; everything else gets Read. Kept local to the layout TU so the
			 * command-buffer anonymous-namespace helpers don't have to move; the two copies are tiny
			 * (<20 lines each) and logically identical. If a third call site ever appears, promote to
			 * a shared header.
			 */
			MTLResourceUsage BucketUsageForBinding(const MetalArgumentBufferBinding& binding)
			{
				switch (binding.Type)
				{
				case GpuParameterType::StorageBuffer:
					return IsWritableBufferType(binding.ObjectType)
						? (MTLResourceUsageRead | MTLResourceUsageWrite)
						: MTLResourceUsageRead;
				case GpuParameterType::StorageTexture:
					return IsWritableTextureType(binding.ObjectType)
						? (MTLResourceUsageRead | MTLResourceUsageWrite)
						: MTLResourceUsageRead;
				default:
					return MTLResourceUsageRead;
				}
			}

			/** B2: @c GpuProgramStageBit mask -> Metal @c MTLRenderStages. Hull/Domain fold into Vertex. */
			MTLRenderStages BucketRenderStagesFromMask(u32 stageMask)
			{
				MTLRenderStages stages = (MTLRenderStages)0;
				if (stageMask & (u32)GpuProgramStageBit::Vertex)
					stages |= MTLRenderStageVertex;
				if (stageMask & (u32)GpuProgramStageBit::Fragment)
					stages |= MTLRenderStageFragment;
				if (stageMask & ((u32)GpuProgramStageBit::Hull | (u32)GpuProgramStageBit::Domain))
					stages |= MTLRenderStageVertex;
				return stages;
			}

			/** Maps an engine texture object type to the Metal texture type used by the argument encoder. */
			MTLTextureType MapMetalTextureType(GpuParameterObjectType type)
			{
				switch (type)
				{
				case GPOT_SAMPLER1D:
				case GPOT_TEXTURE1D:
				case GPOT_RWTEXTURE1D:
					return MTLTextureType1D;
				case GPOT_TEXTURE1DARRAY:
				case GPOT_RWTEXTURE1DARRAY:
					return MTLTextureType1DArray;
				case GPOT_SAMPLER2D:
				case GPOT_TEXTURE2D:
				case GPOT_RWTEXTURE2D:
					return MTLTextureType2D;
				case GPOT_TEXTURE2DARRAY:
				case GPOT_RWTEXTURE2DARRAY:
					return MTLTextureType2DArray;
				case GPOT_SAMPLER2DMS:
				case GPOT_TEXTURE2DMS:
				case GPOT_RWTEXTURE2DMS:
					return MTLTextureType2DMultisample;
				case GPOT_TEXTURE2DMSARRAY:
				case GPOT_RWTEXTURE2DMSARRAY:
					return MTLTextureType2DMultisampleArray;
				case GPOT_SAMPLER3D:
				case GPOT_TEXTURE3D:
				case GPOT_RWTEXTURE3D:
					return MTLTextureType3D;
				case GPOT_SAMPLERCUBE:
				case GPOT_TEXTURECUBE:
					return MTLTextureTypeCube;
				case GPOT_TEXTURECUBEARRAY:
					return MTLTextureTypeCubeArray;
				default:
					return MTLTextureType2D;
				}
			}
		} // namespace

		MetalGpuPipelineParameterSetLayout::MetalGpuPipelineParameterSetLayout(
			MetalGpuDevice& gpuDevice, const GpuProgramParameterDescription& parameterDescription)
			: GpuPipelineParameterSetLayout(parameterDescription)
			, mGpuDevice(gpuDevice)
			, mImpl(B3DMakeUnique<Impl>())
		{
			@autoreleasepool
			{
				// Gather bindings per type, sort deterministically by slot, then assign a monotonically
				// increasing argument-buffer index. Metal argument buffers share one flat index namespace
				// per set across buffers, textures and samplers, so assigning `slot` directly as the index
				// causes collisions (e.g. UB slot 0 and sampler slot 0 would overwrite the same member).
				// The same ordering is replicated in MetalGpuDevice's SPIRV-Cross hook
				// (see fnCollectBinding), so the MSL-side encoder slot agrees with the MTLArgumentEncoder
				// layout built here.
				auto fnCollectBindings = [&](GpuParameterType type)
				{
					const u32 startIndex = (u32)mBindings.Size();
					for (const auto* entry : mUniformsPerType[(u32)type])
					{
						MetalArgumentBufferBinding record;
						record.Slot = entry->Slot;
						record.Type = entry->Type;
						record.ObjectType = entry->ObjectType;
						record.ArraySize = entry->ArraySize;
						record.StageMask = BuildStageMask(entry->Usage);
						mBindings.Add(record);
					}

					// mUniformsPerType entries come from an unordered map in the base class; sort by slot
					// so both sides (layout and shader compile) agree on a canonical order.
					std::sort(mBindings.Data() + startIndex, mBindings.Data() + mBindings.Size(),
						[](const MetalArgumentBufferBinding& a, const MetalArgumentBufferBinding& b)
						{
							return a.Slot < b.Slot;
						});
				};

				// Iterate per-type in the canonical kTypeOrder* sequence from B3DMetalBytecodeLayout.h —
				// the SPIRV-Cross hook in the bsfShaderBackendMSL compiler uses the same constants to assign MSL
				// argument-buffer indices, so both sides must agree on the order by construction.
				static_assert(kTypeOrderUniformBuffer  == 0, "Canonical type order changed; update table below.");
				static_assert(kTypeOrderSampledTexture == 1, "Canonical type order changed; update table below.");
				static_assert(kTypeOrderStorageTexture == 2, "Canonical type order changed; update table below.");
				static_assert(kTypeOrderStorageBuffer  == 3, "Canonical type order changed; update table below.");
				static_assert(kTypeOrderSampler        == 4, "Canonical type order changed; update table below.");
				constexpr GpuParameterType kOrderedTypes[] =
				{
					GpuParameterType::UniformBuffer,	// kTypeOrderUniformBuffer
					GpuParameterType::SampledTexture,	// kTypeOrderSampledTexture
					GpuParameterType::StorageTexture,	// kTypeOrderStorageTexture
					GpuParameterType::StorageBuffer,	// kTypeOrderStorageBuffer
					GpuParameterType::Sampler,			// kTypeOrderSampler
				};
				for (GpuParameterType orderedType : kOrderedTypes)
					fnCollectBindings(orderedType);

				for (u32 i = 0; i < (u32)mBindings.Size(); ++i)
					mBindings[i].ArgIndex = i;

				// Fold every binding's stage mask into one value. Command-buffer bind paths read this to
				// decide which stages receive the argument buffer (B7). Computed after ArgIndex assignment
				// so the layout is fully finalized before the mask snapshots it.
				mCombinedStageMask = 0;
				for (const MetalArgumentBufferBinding& binding : mBindings)
					mCombinedStageMask |= binding.StageMask;

				// B2: group bindings by (usage, render-stage-mask) so the command-buffer residency
				// emission at draw time becomes one @c useResources:count:usage:stages: call per
				// bucket instead of N @c useResource: calls. Samplers don't participate in residency
				// (they are stage-inherent on Metal) so they're filtered out here. Compute buckets
				// drop the stage-mask axis since @c useResources:count:usage: on a compute encoder
				// takes no stage argument.
				auto fnFindOrAddRenderBucket = [&](MTLResourceUsage usage, MTLRenderStages renderStages) -> ArgumentBindingBucket*
				{
					for (auto& bucket : mRenderBuckets)
					{
						if (bucket.Usage == usage && bucket.RenderStages == renderStages)
							return &bucket;
					}
					ArgumentBindingBucket fresh;
					fresh.Usage = usage;
					fresh.RenderStages = renderStages;
					mRenderBuckets.Add(std::move(fresh));
					return &mRenderBuckets[mRenderBuckets.Size() - 1];
				};

				auto fnFindOrAddComputeBucket = [&](MTLResourceUsage usage) -> ArgumentBindingBucket*
				{
					for (auto& bucket : mComputeBuckets)
					{
						if (bucket.Usage == usage)
							return &bucket;
					}
					ArgumentBindingBucket fresh;
					fresh.Usage = usage;
					fresh.RenderStages = (MTLRenderStages)0;
					mComputeBuckets.Add(std::move(fresh));
					return &mComputeBuckets[mComputeBuckets.Size() - 1];
				};

				for (u32 bindingIndex = 0; bindingIndex < (u32)mBindings.Size(); ++bindingIndex)
				{
					const MetalArgumentBufferBinding& binding = mBindings[bindingIndex];
					if (binding.Type == GpuParameterType::Sampler)
						continue;

					const MTLResourceUsage usage = BucketUsageForBinding(binding);
					const MTLRenderStages renderStages = BucketRenderStagesFromMask(binding.StageMask);

					if (renderStages != (MTLRenderStages)0)
					{
						ArgumentBindingBucket* renderBucket = fnFindOrAddRenderBucket(usage, renderStages);
						renderBucket->BindingIndices.Add((u16)bindingIndex);
					}

					if (binding.StageMask & (u32)GpuProgramStageBit::Compute)
					{
						ArgumentBindingBucket* computeBucket = fnFindOrAddComputeBucket(usage);
						computeBucket->BindingIndices.Add((u16)bindingIndex);
					}
				}

				NSMutableArray<MTLArgumentDescriptor*>* descriptors = [NSMutableArray array];
				for (const MetalArgumentBufferBinding& binding : mBindings)
				{
					MTLArgumentDescriptor* desc = [MTLArgumentDescriptor argumentDescriptor];
					desc.index = binding.ArgIndex;
					desc.arrayLength = binding.ArraySize > 1 ? binding.ArraySize : 0;

					switch (binding.Type)
					{
					case GpuParameterType::UniformBuffer:
						desc.dataType = MTLDataTypePointer;
						desc.access = MTLArgumentAccessReadOnly;
						break;
					case GpuParameterType::StorageBuffer:
						desc.dataType = MTLDataTypePointer;
						desc.access = IsWritableBufferType(binding.ObjectType)
							? MTLArgumentAccessReadWrite
							: MTLArgumentAccessReadOnly;
						break;
					case GpuParameterType::SampledTexture:
						desc.dataType = MTLDataTypeTexture;
						desc.textureType = MapMetalTextureType(binding.ObjectType);
						desc.access = MTLArgumentAccessReadOnly;
						break;
					case GpuParameterType::StorageTexture:
						desc.dataType = MTLDataTypeTexture;
						desc.textureType = MapMetalTextureType(binding.ObjectType);
						desc.access = IsWritableTextureType(binding.ObjectType)
							? MTLArgumentAccessReadWrite
							: MTLArgumentAccessReadOnly;
						break;
					case GpuParameterType::Sampler:
						desc.dataType = MTLDataTypeSampler;
						desc.access = MTLArgumentAccessReadOnly;
						break;
					default:
						break;
					}

					[descriptors addObject:desc];
				}

				if ([descriptors count] == 0)
				{
					mArgumentBufferSize = 0;
					mArgumentBufferAlignment = 16;
					return;
				}

				// Retain the descriptor list so each parameter set can mint its own encoder later,
				// then create a transient encoder once up front to read size/alignment — both values
				// are shared across every set that uses this layout.
				mImpl->Descriptors = descriptors;

				id<MTLDevice> device = mGpuDevice.GetMetalDevice();
				id<MTLArgumentEncoder> probeEncoder = [device newArgumentEncoderWithArguments:descriptors];
				if (probeEncoder != nil)
				{
					mArgumentBufferSize = (u64)[probeEncoder encodedLength];
					mArgumentBufferAlignment = (u32)[probeEncoder alignment];
				}
			}
		}

		u32 MetalGpuPipelineParameterSetLayout::GetArgumentBufferIndex(GpuParameterType type, u32 slot) const
		{
			// Linear scan — a parameter set typically has on the order of ten bindings, so this is cheaper
			// than maintaining a map. Note that combined-texture-sampler edge cases are the only way a
			// slot can be shared across types (SampledTexture vs Sampler), which is why we disambiguate
			// on Type here rather than matching on Slot alone.
			for (const MetalArgumentBufferBinding& binding : mBindings)
			{
				if (binding.Type == type && binding.Slot == slot)
					return binding.ArgIndex;
			}

			return (u32)~0u;
		}

		MetalGpuPipelineParameterSetLayout::~MetalGpuPipelineParameterSetLayout()
		{
			if (mImpl)
				mImpl->Descriptors = nil;
		}

		id<MTLArgumentEncoder> MetalGpuPipelineParameterSetLayout::NewArgumentEncoder() const
		{
			if (!mImpl || mImpl->Descriptors == nil || [mImpl->Descriptors count] == 0)
				return nil;

			id<MTLDevice> device = mGpuDevice.GetMetalDevice();
			return [device newArgumentEncoderWithArguments:mImpl->Descriptors];
		}

		MetalGpuPipelineParameterLayout::MetalGpuPipelineParameterLayout(
			MetalGpuDevice& gpuDevice, const GpuPipelineParameterLayoutCreateInformation& createInformation)
			: GpuPipelineParameterLayout(gpuDevice, createInformation)
		{ }
	} // namespace render
} // namespace b3d
