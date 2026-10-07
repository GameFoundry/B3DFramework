//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuExplicitBarrierValidator.h"

#if B3D_GPU_EXPLICIT_BARRIERS && B3D_BUILD_TYPE_DEVELOPMENT

#include "GpuBackend/B3DGpuBackendUtility.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "GpuBackend/B3DGpuPipelineParameterLayout.h"
#include "GpuBackend/B3DGpuPipelineState.h"
#include "GpuBackend/B3DRenderTexture.h"
#include "Utility/B3DBitwise.h"

namespace b3d::render
{
	void GpuExplicitBarrierValidator::ValidateBarriers(const GpuExplicitBarriers& barriers, GpuBarrierPhase phase, const GpuSplitBarrier* split)
	{
		for(const GpuExplicitBufferBarrier& barrier : barriers.BufferBarriers)
		{
			if(barrier.Object == nullptr)
				continue;

			const char* const error = ApplyBarrier(mBuffers[barrier.Object.get()], barrier, phase, split, false);
			B3D_ENSURE_LOG(error == nullptr, "{0} Buffer: '{1}'.", error, barrier.Object->GetName());
		}

		for(const GpuExplicitTextureBarrier& barrier : barriers.TextureBarriers)
		{
			if(barrier.Object != nullptr)
				ValidateImageBarrier(GetTextureSubresources(*barrier.Object, barrier.SubresourceRange), barrier, phase, split);
		}

		for(const GpuExplicitRenderTargetBarrier& barrier : barriers.RenderTargetBarriers)
		{
			if(barrier.Object == nullptr)
				continue;

			// The barrier covers the whole surface, restricted to the aspects it selects
			ImageSubresources image = GetSurfaceSubresources(*barrier.Object, barrier.SurfaceMask);
			image.Range.AspectMask &= barrier.SubresourceRange.AspectMask;
			if(image.Key.Object != nullptr)
				ValidateImageBarrier(image, barrier, phase, split);
		}
	}

	void GpuExplicitBarrierValidator::SetParameterSet(const TShared<GpuParameterSet>& parameters)
	{
		if(parameters == nullptr)
			return;

		const u32 set = parameters->GetSet();
		if(set >= mParameterSets.size())
			mParameterSets.resize(set + 1);

		mParameterSets[set] = parameters;
	}

	void GpuExplicitBarrierValidator::SetVertexBuffers(u32 index, const TShared<GpuBuffer>* buffers, u32 bufferCount)
	{
		if(index + bufferCount > mVertexBuffers.size())
			mVertexBuffers.resize(index + bufferCount);

		for(u32 bufferIndex = 0; bufferIndex < bufferCount; bufferIndex++)
			mVertexBuffers[index + bufferIndex] = buffers[bufferIndex];
	}

	void GpuExplicitBarrierValidator::ValidateRenderPass(const RenderPassCreateInformation& createInformation)
	{
		mRenderPassAttachments.Clear();
		if(createInformation.Target == nullptr)
			return;

		const GpuStageFlags kDepthStencilStages = GpuStageFlag::EarlyFragmentTests | GpuStageFlag::LateFragmentTests;
		const auto fnValidateAttachment = [this, &createInformation](RenderSurfaceMaskBits surface, GpuTextureAspectFlags aspects, GpuStageFlags stages)
		{
			ImageSubresources image = GetSurfaceSubresources(*createInformation.Target, surface);
			image.Range.AspectMask &= aspects;
			if(image.Key.Object == nullptr || !image.Range.AspectMask)
				return;

			// Writable attachments also read, through loads, blending and depth/stencil tests
			const bool readOnly = createInformation.ReadOnlyMask.IsSet(surface);
			const GpuAccessFlags access = readOnly ? GpuAccessFlags(GpuAccessFlag::Read) : GpuAccessFlag::Read | GpuAccessFlag::Write;
			ValidateImageAccess(image, GetAttachmentLayout(surface, createInformation.ReadOnlyMask), stages, access);

			mRenderPassAttachments.Add(image);
		};

		for(u32 colorIndex = 0; colorIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; colorIndex++)
			fnValidateAttachment((RenderSurfaceMaskBits)(RT_COLOR0 << colorIndex), GpuTextureAspectFlag::Color, GpuStageFlag::ColorAttachment);

		// The aspects of a depth/stencil image are read-only separately
		fnValidateAttachment(RT_DEPTH, GpuTextureAspectFlag::Depth, kDepthStencilStages);
		fnValidateAttachment(RT_STENCIL, GpuTextureAspectFlag::Stencil, kDepthStencilStages);
	}

	void GpuExplicitBarrierValidator::ValidateDraw(bool indexed)
	{
		for(const TShared<GpuBuffer>& buffer : mVertexBuffers)
		{
			if(buffer != nullptr)
				ValidateBufferAccess(*buffer, GpuStageFlag::VertexInputAttributes, GpuAccessFlag::Read);
		}

		if(indexed && mIndexBuffer != nullptr)
			ValidateBufferAccess(*mIndexBuffer, GpuStageFlag::VertexInputIndices, GpuAccessFlag::Read);

		if(mGraphicsPipeline != nullptr && mGraphicsPipeline->GetParameterLayout() != nullptr)
			ValidateBindings(*mGraphicsPipeline->GetParameterLayout(), true);
	}

	void GpuExplicitBarrierValidator::ValidateDispatch()
	{
		if(mComputePipeline != nullptr && mComputePipeline->GetParameterLayout() != nullptr)
			ValidateBindings(*mComputePipeline->GetParameterLayout(), false);
	}

	void GpuExplicitBarrierValidator::ValidateBufferAccess(const GpuBuffer& buffer, GpuStageFlags stages, GpuAccessFlags access)
	{
		const char* const error = ApplyAccess(mBuffers[&buffer], GpuImageLayout::Undefined, stages, access);
		B3D_ENSURE_LOG(error == nullptr, "{0} Buffer: '{1}'.", error, buffer.GetName());
	}

	void GpuExplicitBarrierValidator::ValidateTextureAccess(const Texture& texture, const GpuTextureSubresourceRange& range, GpuImageLayout layout, GpuStageFlags stages, GpuAccessFlags access)
	{
		ValidateImageAccess(GetTextureSubresources(texture, range), layout, stages, access);
	}

	void GpuExplicitBarrierValidator::Clear()
	{
		mBuffers.clear();
		mImages.clear();
		mParameterSets.clear();
		mGraphicsPipeline = nullptr;
		mComputePipeline = nullptr;
		mVertexBuffers.clear();
		mIndexBuffer = nullptr;
		mRenderPassAttachments.Clear();
	}

	GpuImageLayout GpuExplicitBarrierValidator::GetAttachmentLayout(RenderSurfaceMaskBits surface, RenderSurfaceMask readOnlyMask)
	{
		if(((u32)surface & (u32)RT_COLOR_ALL) != 0)
			return readOnlyMask.IsSet(surface) ? GpuImageLayout::General : GpuImageLayout::ColorAttachment;

		const bool depthReadOnly = readOnlyMask.IsSet(RT_DEPTH);
		const bool stencilReadOnly = readOnlyMask.IsSet(RT_STENCIL);
		if(depthReadOnly)
			return stencilReadOnly ? GpuImageLayout::DepthStencilReadOnly : GpuImageLayout::DepthReadOnlyStencilAttachment;

		return stencilReadOnly ? GpuImageLayout::DepthAttachmentStencilReadOnly : GpuImageLayout::DepthStencilAttachment;
	}

	GpuExplicitBarrierValidator::ImageSubresources GpuExplicitBarrierValidator::GetTextureSubresources(const Texture& texture, const GpuTextureSubresourceRange& range)
	{
		const TextureProperties& properties = texture.GetProperties();

		ImageSubresources image;
		image.Key.Object = &texture;
		image.FullRange = GpuTextureSubresourceRange(0, properties.MipMapCount + 1, 0, properties.GetFaceCount(), GpuBackendUtility::GetFormatAspects(properties.Format));
		image.Range = GpuBackendUtility::ClampRange(range, image.FullRange);
		image.Name = texture.GetName();

		return image;
	}

	GpuExplicitBarrierValidator::ImageSubresources GpuExplicitBarrierValidator::GetSurfaceSubresources(const RenderTarget& target, RenderSurfaceMaskBits surface)
	{
		const bool isColor = ((u32)surface & (u32)RT_COLOR_ALL) != 0;
		if(target.GetProperties().IsWindow)
		{
			// Windows have one color surface, and their depth and stencil share one image
			if(isColor && surface != RT_COLOR0)
				return ImageSubresources();

			ImageSubresources image;
			image.Key.Object = &target;
			image.Key.Surface = isColor ? 0 : 1;
			image.FullRange = GpuTextureSubresourceRange(0, 1, 0, 1, isColor ? GpuTextureAspectFlags(GpuTextureAspectFlag::Color) : GpuTextureAspectFlag::Depth | GpuTextureAspectFlag::Stencil);
			image.Range = image.FullRange;
			image.Name = "Window";

			return image;
		}

		const RenderTexture& renderTexture = static_cast<const RenderTexture&>(target);
		const RenderSurfaceInformation& surfaceInformation = isColor ? renderTexture.GetColorSurfaceInformation(Bitwise::LeastSignificantBit((u32)surface)) : renderTexture.GetDepthStencilSurfaceInformation();

		if(surfaceInformation.Texture == nullptr)
			return ImageSubresources();

		ImageSubresources image = GetTextureSubresources(*surfaceInformation.Texture, GpuTextureSubresourceRange::AllSubresources());
		image.Range = GpuBackendUtility::GetSurfaceRange(image.FullRange, TextureSurface(surfaceInformation.MipLevel, 1, surfaceInformation.Face, surfaceInformation.FaceCount));

		return image;
	}

	void GpuExplicitBarrierValidator::ValidateBindings(const GpuPipelineParameterLayout& pipelineLayout, bool isDraw)
	{
		for(u32 set = 0; set < std::min(pipelineLayout.GetSetCount(), (u32)mParameterSets.size()); set++)
		{
			const GpuParameterSet* const parameters = mParameterSets[set].get();
			const TShared<GpuPipelineParameterSetLayout>& pipelineSetLayout = pipelineLayout.GetSet(set);
			if(parameters == nullptr || pipelineSetLayout == nullptr)
				continue;

			// Mirrors GpuDrawAccessValidator::AddParameterSet(): only the bindings the pipeline uses are accessed
			const TShared<GpuPipelineParameterSetLayout>& parameterSetLayout = parameters->GetLayout();
			for(u32 typeIndex = 0; typeIndex < (u32)GpuParameterType::Count; typeIndex++)
			{
				const GpuParameterType type = (GpuParameterType)typeIndex;
				if(type == GpuParameterType::Sampler)
					continue;

				for(u32 bindingIndex = 0; bindingIndex < pipelineSetLayout->GetBindingCount(type); bindingIndex++)
				{
					const UniformInformation& pipelineUniform = *pipelineSetLayout->TryGetUniformInformation(type, bindingIndex);
					const UniformInformation* parameterSetUniform = parameterSetLayout->TryGetUniformInformation(pipelineUniform.Slot);
					if(parameterSetUniform == nullptr || parameterSetUniform->Type != type)
						continue;

					const GpuResourceUseFlags stageUse = GpuBackendUtility::GetShaderResourceUseFlags(pipelineUniform.Usage);
					for(u32 arrayIndex = 0; arrayIndex < std::min(pipelineUniform.ArraySize, parameterSetUniform->ArraySize); arrayIndex++)
					{
						switch(type)
						{
						case GpuParameterType::UniformBuffer:
							if(const TShared<GpuBuffer>& buffer = parameters->GetUniformBuffer(pipelineUniform.Slot, arrayIndex))
								ValidateBufferAccess(*buffer, GpuBackendUtility::GetStageFlags(stageUse | GpuResourceUseFlag::UniformBuffer), GpuAccessFlag::Read);

							break;
						case GpuParameterType::StorageBuffer:
							if(const TShared<GpuBuffer>& buffer = parameters->GetStorageBuffer(pipelineUniform.Slot, arrayIndex))
							{
								const GpuAccessFlags access = GpuObjectParameterTypeInformation::IsReadWriteBuffer(pipelineUniform.ObjectType) ? GpuAccessFlag::Read | GpuAccessFlag::Write : GpuAccessFlags(GpuAccessFlag::Read);
								ValidateBufferAccess(*buffer, GpuBackendUtility::GetStageFlags(stageUse | GpuResourceUseFlag::ShaderAccess), access);
							}
							break;
						case GpuParameterType::SampledTexture:
							if(const TShared<Texture>& texture = parameters->GetSampledTexture(pipelineUniform.Slot, arrayIndex))
							{
								ImageSubresources image = GetTextureSubresources(*texture, GpuTextureSubresourceRange::AllSubresources());
								image.Range = GpuBackendUtility::GetSurfaceRange(image.FullRange, parameters->GetTextureSurface(pipelineUniform.Slot, arrayIndex));

								// Sampled depth/stencil views only read the depth aspect
								if(image.Range.AspectMask.IsSet(GpuTextureAspectFlag::Depth))
									image.Range.AspectMask = GpuTextureAspectFlag::Depth;

								// A sampled attachment is in the layout of the render pass, which checked it
								const GpuImageLayout layout = isDraw && IsRenderPassAttachment(image) ? GpuImageLayout::Undefined : GpuImageLayout::ShaderReadOnly;
								ValidateImageAccess(image, layout, GpuBackendUtility::GetStageFlags(stageUse | GpuResourceUseFlag::ShaderAccess), GpuAccessFlag::Read);
							}
							break;
						case GpuParameterType::StorageTexture:
							if(const TShared<Texture>& texture = parameters->GetStorageTexture(pipelineUniform.Slot, arrayIndex))
							{
								ImageSubresources image = GetTextureSubresources(*texture, GpuTextureSubresourceRange::AllSubresources());
								image.Range = GpuBackendUtility::GetSurfaceRange(image.FullRange, parameters->GetStorageTextureSurface(pipelineUniform.Slot, arrayIndex));
								ValidateImageAccess(image, GpuImageLayout::General, GpuBackendUtility::GetStageFlags(stageUse | GpuResourceUseFlag::ShaderAccess), GpuAccessFlag::Read | GpuAccessFlag::Write);
							}
							break;
						default:
							break;
						}
					}
				}
			}
		}
	}

	void GpuExplicitBarrierValidator::ValidateImageBarrier(const ImageSubresources& image, const GpuExplicitBarrier& barrier, GpuBarrierPhase phase, const GpuSplitBarrier* split)
	{
		const char* const error = ForEachSubresourceState(image, [&](ResourceState& state)
		{
			return ApplyBarrier(state, barrier, phase, split, true);
		});

		B3D_ENSURE_LOG(error == nullptr, "{0} Image: '{1}'.", error, image.Name);
	}

	void GpuExplicitBarrierValidator::ValidateImageAccess(const ImageSubresources& image, GpuImageLayout layout, GpuStageFlags stages, GpuAccessFlags access)
	{
		const char* const error = ForEachSubresourceState(image, [&](ResourceState& state)
		{
			return ApplyAccess(state, layout, stages, access);
		});

		B3D_ENSURE_LOG(error == nullptr, "{0} Image: '{1}'.", error, image.Name);
	}

	bool GpuExplicitBarrierValidator::IsRenderPassAttachment(const ImageSubresources& image) const
	{
		return std::any_of(mRenderPassAttachments.begin(), mRenderPassAttachments.end(), [&image](const ImageSubresources& attachment)
		{
			return attachment.Key == image.Key && GpuBackendUtility::RangeOverlaps(attachment.Range, image.Range);
		});
	}

	const char* GpuExplicitBarrierValidator::ApplyBarrier(ResourceState& state, const GpuExplicitBarrier& barrier, GpuBarrierPhase phase, const GpuSplitBarrier* split, bool isImage)
	{
		const char* error = nullptr;
		const GpuAccessState& source = barrier.Source;
		if(state.PendingAcquire != nullptr)
		{
			// The release already checked the source
			if(phase != GpuBarrierPhase::Acquire || state.PendingAcquire != split)
				error = "An explicit barrier was recorded between the release and the acquire of a split barrier of the same resource.";
		}
		else if(barrier.Flags.IsSet(GpuBarrierFlag::AliasAcquire) && (state.IsDeclared || state.UsedAccess != GpuAccessFlag::None))
			error = "An alias acquire must precede every other use of the resource on the command buffer.";
		else if(isImage && state.IsDeclared && source.Layout != GpuImageLayout::Undefined && source.Layout != state.Declared.Layout)
			error = "The source layout of an explicit barrier differs from the destination layout of the previous barrier of the resource.";
		else if(!source.Stages.IsSetAll(state.UsedStages))
			error = "The source of an explicit barrier is missing stages that accessed the resource since the previous barrier.";
		else if(state.UsedAccess.IsSet(GpuAccessFlag::Write) && !source.Access.IsSet(GpuAccessFlag::Write))
			error = "The source of an explicit barrier is missing the writes of the resource since the previous barrier.";

		if(phase == GpuBarrierPhase::Release)
			state.PendingAcquire = split;
		else
		{
			state.PendingAcquire = nullptr;
			state.Declared = barrier.Destination;
			state.IsDeclared = true;
		}

		state.UsedStages = GpuStageFlag::None;
		state.UsedAccess = GpuAccessFlag::None;
		return error;
	}

	const char* GpuExplicitBarrierValidator::ApplyAccess(ResourceState& state, GpuImageLayout layout, GpuStageFlags stages, GpuAccessFlags access)
	{
		const char* error = nullptr;
		if(state.PendingAcquire != nullptr)
			error = "The resource was accessed between the release and the acquire of a split barrier.";
		else if(state.IsDeclared && !state.Declared.Stages.IsSetAll(stages))
			error = "The resource was accessed in stages that its last explicit barrier does not declare.";
		else if(state.IsDeclared && !state.Declared.Access.IsSetAll(access))
			error = "The resource was accessed with reads or writes that its last explicit barrier does not declare.";
		else if(state.IsDeclared && layout != GpuImageLayout::Undefined && layout != state.Declared.Layout)
			error = "The image was accessed in a different layout than its last explicit barrier declares.";

		state.UsedStages |= stages;
		state.UsedAccess |= access;
		return error;
	}

	template<class TFunction>
	const char* GpuExplicitBarrierValidator::ForEachSubresourceState(const ImageSubresources& image, TFunction&& function)
	{
		const GpuTextureSubresourceRange& fullRange = image.FullRange;
		const GpuTextureSubresourceRange& clampedRange = image.Range;

		Vector<ResourceState>& states = mImages[image.Key];
		if(states.empty())
			states.resize(fullRange.MipLevelCount * fullRange.ArrayLayerCount * fullRange.GetAspectCount());

		const char* firstError = nullptr;
		u32 aspectIndex = 0;
		for(GpuTextureAspectFlag aspect : kGpuTextureAspects)
		{
			if(!fullRange.AspectMask.IsSet(aspect))
				continue;

			if(clampedRange.AspectMask.IsSet(aspect))
			{
				for(u32 face = clampedRange.BaseArrayLayer; face < clampedRange.BaseArrayLayer + clampedRange.ArrayLayerCount; face++)
				{
					for(u32 mip = clampedRange.BaseMipLevel; mip < clampedRange.BaseMipLevel + clampedRange.MipLevelCount; mip++)
					{
						const u32 index = (aspectIndex * fullRange.ArrayLayerCount + face) * fullRange.MipLevelCount + mip;
						const char* const error = function(states[index]);
						if(firstError == nullptr)
							firstError = error;
					}
				}
			}

			aspectIndex++;
		}

		return firstError;
	}
} // namespace b3d::render

#endif
