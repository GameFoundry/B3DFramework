//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalGpuPipelineParameterLayout.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalShaderABI.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"
#include "Utility/B3DCommonTypes.h"
#include "Debug/B3DLog.h"
#include <algorithm>

namespace b3d
{
	namespace render
	{
		namespace
		{
			u64 AlignUp(u64 value, u32 alignment)
			{
				const u64 mask = (u64)alignment - 1;
				return (value + mask) & ~mask;
			}

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

		} // namespace

		MetalGpuPipelineParameterSetLayout::MetalGpuPipelineParameterSetLayout(
			const GpuProgramParameterDescription& parameterDescription,
			const TShared<GpuResourceTableLayout>& resourceTableLayout, u32 tableIndex)
			: GpuPipelineParameterSetLayout(parameterDescription)
		{
			// Single-table path: apply the same reflected table to every stage that references the set.
			// Correct only when the stages agree on packing; MetalGpuPipelineParameterLayout rebuilds
			// with genuine per-stage tables right after construction.
			Array<StageReflectedTable, kMetalStageSectionCount> stageTables;
			for (u32 sectionIndex = 0; sectionIndex < kMetalStageSectionCount; sectionIndex++)
			{
				stageTables[sectionIndex].Layout = resourceTableLayout;
				stageTables[sectionIndex].TableIndex = tableIndex;
			}

			Build(stageTables);
		}

		MetalGpuPipelineParameterSetLayout::MetalGpuPipelineParameterSetLayout(
			const GpuProgramParameterDescription& parameterDescription,
			const Array<StageReflectedTable, kMetalStageSectionCount>& stageTables)
			: GpuPipelineParameterSetLayout(parameterDescription)
		{
			Build(stageTables);
		}

		void MetalGpuPipelineParameterSetLayout::Build(const Array<StageReflectedTable, kMetalStageSectionCount>& stageTables)
		{
			mBindings.Clear();
			mRenderBuckets.Clear();
			mComputeBuckets.Clear();
			mArgumentBufferSize = 0;
			mCombinedStageMask = 0;
			for (u32 sectionIndex = 0; sectionIndex < kMetalStageSectionCount; sectionIndex++)
			{
				mStageSectionBases[sectionIndex] = 0;
				mStageSectionSizes[sectionIndex] = 0;
			}

			@autoreleasepool
			{
				// Gather bindings per type and sort deterministically. This is both the CPU dirty-slot order
				// and the fallback ABI for explicitly-created layouts that have no program reflection attached.
				auto fnCollectBindings = [&](GpuParameterType type)
				{
					const u32 startIndex = (u32)mBindings.Size();
					for (const auto* entry : mUniformsPerType[(u32)type])
					{
						if (entry->Slot > kMetalMaximumArgumentBufferSlot || entry->ArraySize == 0
							|| entry->ArraySize > kMetalArgumentBufferArrayStride)
						{
							B3D_LOG(Error, LogRenderBackend,
								"Metal argument-buffer binding exceeds the supported ABI. Slot: {0}, array size: {1}.",
								entry->Slot, entry->ArraySize);
							continue;
						}

						MetalArgumentBufferBinding record;
						record.Slot = entry->Slot;
						record.Type = entry->Type;
						record.ObjectType = entry->ObjectType;
						record.ArraySize = entry->ArraySize;
						record.DynamicOffsetIndex = entry->DynamicOffsetIndex;
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

				// Iterate per-type in the canonical kTypeOrder* sequence from B3DMetalShaderABI.h so
				// CPU-side dirty-slot indices remain deterministic across layouts and shader permutations.
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

				// Assign dense CPU-side indices plus the canonical fallback packing (used by stages that
				// reference the set without a reflected table — explicitly created layouts and tests).
				u32 resourceIndex = 0;
				u32 fallbackByteOffset = 0;
				TInlineArray<u32, 16> fallbackOffsets;
				for (MetalArgumentBufferBinding& binding : mBindings)
				{
					binding.ArgIndex = resourceIndex;
					binding.FirstResourceIndex = resourceIndex;
					resourceIndex += binding.ArraySize;
					fallbackOffsets.Add(fallbackByteOffset);
					fallbackByteOffset += binding.ArraySize * (u32)sizeof(u64);
				}

				// Maps GetMetalStageSectionIndex slots back to their stage bits.
				constexpr GpuProgramStageBit kSectionStageBits[kMetalStageSectionCount] =
				{
					GpuProgramStageBit::Vertex,
					GpuProgramStageBit::Fragment,
					GpuProgramStageBit::Compute,
				};

				// Lay out one argument-buffer section per referencing stage. Every stage compiles its own
				// MSL struct and per-stage dead-resource stripping means the structs can pack differently,
				// so each stage reads its own section; CommitPendingBindings writes each binding into every
				// section that declares it. Stages without a reflected table share one canonical section.
				bool reflectedLayoutValid = true;
				u64 nextSectionBase = 0;
				bool fallbackSectionAssigned = false;
				u64 fallbackSectionBase = 0;
				for (u32 sectionIndex = 0; sectionIndex < kMetalStageSectionCount; sectionIndex++)
				{
					const u32 stageBit = (u32)kSectionStageBits[sectionIndex];

					bool stageReferenced = false;
					for (const MetalArgumentBufferBinding& binding : mBindings)
						stageReferenced |= (binding.StageMask & stageBit) != 0;
					if (!stageReferenced)
						continue;

					const TShared<GpuResourceTableLayout>& stageLayout = stageTables[sectionIndex].Layout;
					const u32 stageTableIndex = stageTables[sectionIndex].TableIndex;
					const GpuDescriptorTable* reflectedTable = nullptr;
					if (stageLayout != nullptr && stageTableIndex < (u32)stageLayout->Tables.size())
						reflectedTable = &stageLayout->Tables[stageTableIndex];

					if (reflectedTable == nullptr)
					{
						// Canonical fallback packing, shared by every unreflected stage.
						if (!fallbackSectionAssigned)
						{
							fallbackSectionBase = AlignUp(nextSectionBase, mArgumentBufferAlignment);
							nextSectionBase = fallbackSectionBase + fallbackByteOffset;
							fallbackSectionAssigned = true;
						}

						mStageSectionBases[sectionIndex] = fallbackSectionBase;
						mStageSectionSizes[sectionIndex] = fallbackByteOffset;

						u32 bindingIndex = 0;
						for (MetalArgumentBufferBinding& binding : mBindings)
						{
							if ((binding.StageMask & stageBit) != 0)
							{
								binding.StageByteOffsets[sectionIndex] = fallbackOffsets[bindingIndex];
								binding.StageByteStrides[sectionIndex] = sizeof(u64);
							}
							bindingIndex++;
						}
						continue;
					}

					mStageSectionBases[sectionIndex] = AlignUp(nextSectionBase, mArgumentBufferAlignment);
					mStageSectionSizes[sectionIndex] = reflectedTable->SizeInBytes;
					nextSectionBase = mStageSectionBases[sectionIndex] + reflectedTable->SizeInBytes;

					for (MetalArgumentBufferBinding& binding : mBindings)
					{
						if ((binding.StageMask & stageBit) == 0)
							continue;

						const GpuDescriptorTableEntry* reflectedEntry = nullptr;
						for(const GpuDescriptorTableEntry& candidate : stageLayout->GetEntries(*reflectedTable))
						{
							if(candidate.Kind == GpuDescriptorEntryKind::Resource && candidate.Type == binding.Type
								&& candidate.Slot == binding.Slot)
							{
								reflectedEntry = &candidate;
								break;
							}
						}

						if(reflectedEntry == nullptr || reflectedEntry->DescriptorCount != binding.ArraySize
							|| reflectedEntry->DescriptorSizeInBytes < sizeof(u64))
						{
							B3D_LOG(Error, LogRenderBackend, "Metal reflection is missing a valid Tier-2 argument-buffer entry "
								"for set {0}, slot {1}, type {2}.", reflectedTable->Set, binding.Slot, (u32)binding.Type);
							reflectedLayoutValid = false;
							continue;
						}

						const u64 bindingEnd = (u64)reflectedEntry->OffsetInBytes
							+ (u64)(binding.ArraySize - 1) * reflectedEntry->DescriptorSizeInBytes + sizeof(u64);
						if(bindingEnd > reflectedTable->SizeInBytes)
						{
							B3D_LOG(Error, LogRenderBackend, "Metal reflection reported an out-of-bounds argument-buffer entry "
								"for set {0}, slot {1}.", reflectedTable->Set, binding.Slot);
							reflectedLayoutValid = false;
							continue;
						}

						binding.StageByteOffsets[sectionIndex] = reflectedEntry->OffsetInBytes;
						binding.StageByteStrides[sectionIndex] = reflectedEntry->DescriptorSizeInBytes;
					}
				}

				if(!reflectedLayoutValid)
					return;

				mArgumentBufferSize = AlignUp(nextSectionBase, mArgumentBufferAlignment);

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

				for (const MetalArgumentBufferBinding& binding : mBindings)
				{
					if (binding.Type == GpuParameterType::Sampler)
						continue;

					const MTLResourceUsage usage = BucketUsageForBinding(binding);
					const MTLRenderStages renderStages = BucketRenderStagesFromMask(binding.StageMask);

					if (renderStages != (MTLRenderStages)0)
					{
						ArgumentBindingBucket* renderBucket = fnFindOrAddRenderBucket(usage, renderStages);
						for (u32 arrayIndex = 0; arrayIndex < binding.ArraySize; arrayIndex++)
							renderBucket->ResourceIndices.Add(binding.FirstResourceIndex + arrayIndex);
					}

					if (binding.StageMask & (u32)GpuProgramStageBit::Compute)
					{
						ArgumentBindingBucket* computeBucket = fnFindOrAddComputeBucket(usage);
						for (u32 arrayIndex = 0; arrayIndex < binding.ArraySize; arrayIndex++)
							computeBucket->ResourceIndices.Add(binding.FirstResourceIndex + arrayIndex);
					}
				}

			}
		}

		u32 MetalGpuPipelineParameterSetLayout::GetArgumentBufferIndex(GpuParameterType type, u32 slot, u32 arrayIndex) const
		{
			// Linear scan — a parameter set typically has on the order of ten bindings, so this is cheaper
			// than maintaining a map. Note that combined-texture-sampler edge cases are the only way a
			// slot can be shared across types (SampledTexture vs Sampler), which is why we disambiguate
			// on Type here rather than matching on Slot alone.
			for (const MetalArgumentBufferBinding& binding : mBindings)
			{
				if (binding.Type == type && binding.Slot == slot)
					return arrayIndex < binding.ArraySize ? binding.ArgIndex + arrayIndex : (u32)~0u;
			}

			return (u32)~0u;
		}

		const MetalArgumentBufferBinding* MetalGpuPipelineParameterSetLayout::FindBinding(GpuParameterType type, u32 slot) const
		{
			for (const MetalArgumentBufferBinding& binding : mBindings)
			{
				if (binding.Type == type && binding.Slot == slot)
					return &binding;
			}

			return nullptr;
		}

		u32 MetalGpuPipelineParameterSetLayout::GetResourceIndex(GpuParameterType type, u32 slot, u32 arrayIndex) const
		{
			for (const MetalArgumentBufferBinding& binding : mBindings)
			{
				if (binding.Type == type && binding.Slot == slot)
					return arrayIndex < binding.ArraySize ? binding.FirstResourceIndex + arrayIndex : (u32)~0u;
			}

			return (u32)~0u;
		}

		bool MetalGpuPipelineParameterSetLayout::GetDynamicOffsetBinding(u32 dynamicOffsetIndex,
			GpuParameterType& type, u32& slot, u32& arrayIndex) const
		{
			for (const MetalArgumentBufferBinding& binding : mBindings)
			{
				if (binding.DynamicOffsetIndex == (u32)~0u || dynamicOffsetIndex < binding.DynamicOffsetIndex
					|| dynamicOffsetIndex >= binding.DynamicOffsetIndex + binding.ArraySize)
					continue;

				type = binding.Type;
				slot = binding.Slot;
				arrayIndex = dynamicOffsetIndex - binding.DynamicOffsetIndex;
				return true;
			}

			return false;
		}

		namespace
		{
			/**
			 * Locates the reflected descriptor table backing @p set within a program's resource-table
			 * layout — the child table referenced by a root-table SubTable entry whose set matches.
			 * Mirrors the equivalent walk in the generic GpuPipelineParameterLayout constructor.
			 */
			u32 FindSetTable(const GpuResourceTableLayout& layout, u32 set)
			{
				if(layout.IsEmpty())
					return ~0u;

				for(const GpuDescriptorTableEntry& entry : layout.GetEntries(layout.GetRootTable()))
				{
					if(entry.Kind != GpuDescriptorEntryKind::SubTable)
						continue;

					if(layout.Tables[entry.TableIndex].Set == set)
						return entry.TableIndex;
				}

				return ~0u;
			}
		} // namespace

		MetalGpuPipelineParameterLayout::MetalGpuPipelineParameterLayout(
			MetalGpuDevice& gpuDevice, const GpuPipelineParameterLayoutCreateInformation& createInformation)
			: GpuPipelineParameterLayout(gpuDevice, createInformation)
		{
			if(createInformation.Vertex != nullptr && createInformation.Vertex->PushConstantBufferSize != 0)
				mPushConstantStageMask |= (u32)GpuProgramStageBit::Vertex;
			if(createInformation.Fragment != nullptr && createInformation.Fragment->PushConstantBufferSize != 0)
				mPushConstantStageMask |= (u32)GpuProgramStageBit::Fragment;
			if(createInformation.Compute != nullptr && createInformation.Compute->PushConstantBufferSize != 0)
				mPushConstantStageMask |= (u32)GpuProgramStageBit::Compute;

			// The generic constructor hands each set a single stage's reflected table, which silently
			// assumes every stage packs the set identically. Per-stage dead-resource stripping breaks that
			// assumption (a vertex program that never samples a texture packs a smaller struct than its
			// fragment program), so rebuild each set's stage sections from the genuine per-stage tables.
			for (u32 set = 0; set < GetSetCount(); set++)
			{
				auto metalSet = std::static_pointer_cast<MetalGpuPipelineParameterSetLayout>(mSets[set]);
				if (metalSet == nullptr)
					continue;

				Array<MetalGpuPipelineParameterSetLayout::StageReflectedTable, kMetalStageSectionCount> stageTables;
				bool anyStageTable = false;
				for (u32 programIndex = 0; programIndex < GPT_COUNT; programIndex++)
				{
					const TShared<GpuResourceTableLayout>& stageLayout = createInformation.ResourceTableLayouts[programIndex];
					if (stageLayout == nullptr)
						continue;

					const u32 sectionIndex = GetMetalStageSectionIndex((GpuProgramStageBit)(1u << programIndex));
					if (sectionIndex == ~0u)
						continue;

					const u32 tableIndex = FindSetTable(*stageLayout, set);
					if (tableIndex == ~0u)
						continue;

					stageTables[sectionIndex].Layout = stageLayout;
					stageTables[sectionIndex].TableIndex = tableIndex;
					anyStageTable = true;
				}

				if (anyStageTable)
					metalSet->RebuildWithStageTables(stageTables);
			}
		}
	} // namespace render
} // namespace b3d
