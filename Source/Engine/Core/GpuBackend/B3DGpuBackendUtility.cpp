//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "GpuBackend/B3DRenderTexture.h"
#include "GpuBackend/B3DRenderWindow.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuFramebuffer.h"
#include "Image/B3DTexture.h"

#include "Utility/B3DBitwise.h"
#include "Math/B3DMath.h"
#include "Image/B3DPixelUtility.h"

using namespace b3d;
using namespace b3d::render;

namespace
{
	void CutHorizontalRange(const GpuTextureSubresourceRange& toCut, const GpuTextureSubresourceRange& cutWith, GpuTextureSubresourceRange* output, u32& numAreas)
	{
		numAreas = 0;

		i32 leftCut = Math::Clamp((i32)cutWith.BaseArrayLayer - (i32)toCut.BaseArrayLayer, 0, (i32)toCut.ArrayLayerCount);
		i32 rightCut = Math::Clamp((i32)(cutWith.BaseArrayLayer + cutWith.ArrayLayerCount) - (i32)toCut.BaseArrayLayer, 0, (i32)toCut.ArrayLayerCount);

		if(leftCut > 0 && leftCut < (i32)toCut.ArrayLayerCount)
		{
			output[numAreas] = toCut;
			GpuTextureSubresourceRange& range = output[numAreas];

			range.BaseArrayLayer = toCut.BaseArrayLayer;
			range.ArrayLayerCount = leftCut;

			numAreas++;
		}

		if(rightCut > 0 && rightCut < (i32)toCut.ArrayLayerCount)
		{
			output[numAreas] = toCut;
			GpuTextureSubresourceRange& range = output[numAreas];

			range.BaseArrayLayer = toCut.BaseArrayLayer + rightCut;
			range.ArrayLayerCount = toCut.ArrayLayerCount - rightCut;

			numAreas++;
		}

		// Middle cut
		if(rightCut > leftCut)
		{
			output[numAreas] = toCut;
			GpuTextureSubresourceRange& range = output[numAreas];

			range.BaseArrayLayer = toCut.BaseArrayLayer + leftCut;
			range.ArrayLayerCount = toCut.ArrayLayerCount - (toCut.ArrayLayerCount - rightCut) - leftCut;

			numAreas++;
		}

		// Nothing to cut
		if(numAreas == 0)
		{
			output[numAreas] = toCut;
			numAreas++;
		}
	}

	void CutVerticalRange(const GpuTextureSubresourceRange& toCut, const GpuTextureSubresourceRange& cutWith, GpuTextureSubresourceRange* output, u32& numAreas)
	{
		numAreas = 0;

		i32 topCut = Math::Clamp((i32)cutWith.BaseMipLevel - (i32)toCut.BaseMipLevel, 0, (i32)toCut.MipLevelCount);
		i32 bottomCut = Math::Clamp((i32)(cutWith.BaseMipLevel + cutWith.MipLevelCount) - (i32)toCut.BaseMipLevel, 0, (i32)toCut.MipLevelCount);

		if(topCut > 0 && topCut < (i32)toCut.MipLevelCount)
		{
			output[numAreas] = toCut;
			GpuTextureSubresourceRange& range = output[numAreas];

			range.BaseMipLevel = toCut.BaseMipLevel;
			range.MipLevelCount = topCut;

			numAreas++;
		}

		if(bottomCut > 0 && bottomCut < (i32)toCut.MipLevelCount)
		{
			output[numAreas] = toCut;
			GpuTextureSubresourceRange& range = output[numAreas];

			range.BaseMipLevel = toCut.BaseMipLevel + bottomCut;
			range.MipLevelCount = toCut.MipLevelCount - bottomCut;

			numAreas++;
		}

		// Middle cut
		if(bottomCut > topCut)
		{
			output[numAreas] = toCut;
			GpuTextureSubresourceRange& range = output[numAreas];

			range.BaseMipLevel = toCut.BaseMipLevel + topCut;
			range.MipLevelCount = toCut.MipLevelCount - (toCut.MipLevelCount - bottomCut) - topCut;

			numAreas++;
		}

		// Nothing to cut
		if(numAreas == 0)
		{
			output[numAreas] = toCut;
			numAreas++;
		}
	}

#if B3D_BUILD_TYPE_DEVELOPMENT
	/** Returns the surfaces @p target has. */
	RenderSurfaceMask GetTargetSurfaces(const render::RenderTarget& target)
	{
		RenderSurfaceMask output = RT_NONE;
		if(target.GetProperties().IsWindow)
		{
			output = RT_COLOR0;
			if(static_cast<const render::RenderWindow&>(target).HasDepthBuffer())
				output |= RT_DEPTH_STENCIL;

			return output;
		}

		const auto& renderTexture = static_cast<const render::RenderTexture&>(target);
		for(u32 colorIndex = 0; colorIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; colorIndex++)
		{
			if(renderTexture.GetColorSurfaceInformation(colorIndex).Texture != nullptr)
				output |= (RenderSurfaceMaskBits)(RT_COLOR0 << colorIndex);
		}

		if(renderTexture.GetDepthStencilSurfaceInformation().Texture != nullptr)
			output |= RT_DEPTH_STENCIL;

		return output;
	}

	/** Sets the clear values of the surfaces in @p poisonMask to values that stand out: color to magenta, depth and stencil to 0. */
	void SetPoisonClearValues(RenderSurfaceMask poisonMask, RenderTargetClearValues& clearValues)
	{
		for(u32 colorIndex = 0; colorIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; colorIndex++)
		{
			if(poisonMask.IsSet((RenderSurfaceMaskBits)(RT_COLOR0 << colorIndex)))
				clearValues.Colors[colorIndex] = Color(1.0f, 0.0f, 1.0f, 1.0f);
		}

		if(poisonMask.IsSet(RT_DEPTH))
			clearValues.Depth = 0.0f;

		if(poisonMask.IsSet(RT_STENCIL))
			clearValues.Stencil = 0;
	}
#endif
}

GpuResourceUseFlags GpuBackendUtility::GetShaderResourceUseFlags(GpuProgramStageBits stages)
{
	GpuResourceUseFlags output = GpuResourceUseFlag::Undefined;
	if(stages.IsSetAny(GpuProgramStageBit::Vertex | GpuProgramStageBit::Hull | GpuProgramStageBit::Domain | GpuProgramStageBit::Geometry))
		output |= GpuResourceUseFlag::StageVertexShader;

	if(stages.IsSet(GpuProgramStageBit::Fragment))
		output |= GpuResourceUseFlag::StageFragmentShader;

	if(stages.IsSet(GpuProgramStageBit::Compute))
		output |= GpuResourceUseFlag::StageComputeShader;

	return output;
}

GpuStageFlags GpuBackendUtility::GetStageFlags(GpuResourceUseFlags usage)
{
	GpuStageFlags accessStageFlags;

	if(usage.IsSetAny(GpuResourceUseFlag::ShaderAccess))
	{
		if(usage.IsSet(GpuResourceUseFlag::StageVertexShader))
			accessStageFlags |= GpuStageFlag::VertexShaderNonUniform;

		if(usage.IsSet(GpuResourceUseFlag::StageFragmentShader))
			accessStageFlags |= GpuStageFlag::FragmentShaderNonUniform;

		if(usage.IsSet(GpuResourceUseFlag::StageComputeShader))
			accessStageFlags |= GpuStageFlag::ComputeShaderNonUniform;

		// Assume all stages if none are set explicitly
		if(!usage.IsSetAny(GpuResourceUseFlag::AnyStage))
			accessStageFlags |= GpuStageFlag::VertexShaderNonUniform | GpuStageFlag::FragmentShaderNonUniform | GpuStageFlag::ComputeShaderNonUniform;
	}

	if(usage.IsSet(GpuResourceUseFlag::IndexBuffer))
		accessStageFlags |= GpuStageFlag::VertexInputIndices;

	if(usage.IsSet(GpuResourceUseFlag::VertexBuffer))
		accessStageFlags |= GpuStageFlag::VertexInputAttributes;

	if(usage.IsSet(GpuResourceUseFlag::UniformBuffer))
	{
		if(usage.IsSet(GpuResourceUseFlag::StageVertexShader))
			accessStageFlags |= GpuStageFlag::VertexShaderUniform;

		if(usage.IsSet(GpuResourceUseFlag::StageFragmentShader))
			accessStageFlags |= GpuStageFlag::FragmentShaderUniform;

		if(usage.IsSet(GpuResourceUseFlag::StageComputeShader))
			accessStageFlags |= GpuStageFlag::ComputeShaderUniform;

		// Assume all stages if none are set explicitly
		if(!usage.IsSetAny(GpuResourceUseFlag::AnyStage))
			accessStageFlags |= GpuStageFlag::VertexShaderUniform | GpuStageFlag::FragmentShaderUniform | GpuStageFlag::ComputeShaderUniform;
	}

	if(usage.IsSet(GpuResourceUseFlag::Transfer))
		accessStageFlags |= GpuStageFlag::Transfer;

	if(usage.IsSet(GpuResourceUseFlag::Resolve))
		accessStageFlags |= GpuStageFlag::Resolve;

	if(usage.IsSet(GpuResourceUseFlag::Host))
		accessStageFlags |= GpuStageFlag::Host;

	if(usage.IsSet(GpuResourceUseFlag::ColorAttachment))
		accessStageFlags |= GpuStageFlag::ColorAttachment;

	if(usage.IsSet(GpuResourceUseFlag::DepthStencilAttachment))
		accessStageFlags |= GpuStageFlag::EarlyFragmentTests | GpuStageFlag::LateFragmentTests;

	return accessStageFlags;
}

GpuStageFlags GpuBackendUtility::GetQueueStageFlags(GpuQueueType queueType)
{
	switch(queueType)
	{
	case GQT_COMPUTE:
		return GpuStageFlag::ComputeShaderUniform | GpuStageFlag::ComputeShaderNonUniform | GpuStageFlag::DrawIndirect | GpuStageFlag::Transfer | GpuStageFlag::Host;
	case GQT_TRANSFER:
		return GpuStageFlag::Transfer | GpuStageFlag::Host;
	default:
		return GpuStageFlag::All;
	}
}

bool GpuBackendUtility::IsSampleableOnly(TextureUsageFlags usage)
{
	return !usage.IsSetAny(TextureUsageFlag::RenderTarget | TextureUsageFlag::DepthStencil | TextureUsageFlag::AllowUnorderedAccessOnTheGPU | TextureUsageFlag::StoreOnCPUWithGPUAccess);
}

bool GpuBackendUtility::IsConcurrentlyReadable(TextureUsageFlags usage)
{
	return usage.IsSet(TextureUsageFlag::AllowConcurrentQueueReads) || IsSampleableOnly(usage);
}

const char* GpuBackendUtility::GetAccessStageName(GpuStageFlag flag)
{
	switch(flag)
	{
	case GpuStageFlag::None: return "None";
	case GpuStageFlag::DrawIndirect: return "DrawIndirect";
	case GpuStageFlag::VertexInputAttributes: return "VertexInputAttributes";
	case GpuStageFlag::VertexInputIndices: return "VertexInputIndices";
	case GpuStageFlag::VertexShaderNonUniform: return "VertexShaderNonUniform";
	case GpuStageFlag::FragmentShaderNonUniform: return "FragmentShaderNonUniform";
	case GpuStageFlag::ComputeShaderNonUniform: return "ComputeShaderNonUniform";
	case GpuStageFlag::VertexShaderUniform: return "VertexShaderUniform";
	case GpuStageFlag::FragmentShaderUniform: return "FragmentShaderUniform";
	case GpuStageFlag::ComputeShaderUniform: return "ComputeShaderUniform";
	case GpuStageFlag::EarlyFragmentTests: return "EarlyFragmentTests";
	case GpuStageFlag::LateFragmentTests: return "LateFragmentTests";
	case GpuStageFlag::ColorAttachment: return "ColorAttachment";
	case GpuStageFlag::Transfer: return "Transfer";
	case GpuStageFlag::Host: return "Host";
	case GpuStageFlag::Resolve: return "Resolve";
	case GpuStageFlag::AllShader: return "AllShader";
	case GpuStageFlag::All: return "All";
	default: return "Unknown";
	}
}

void GpuBackendUtility::GetAccessStageNames(GpuStageFlags flags, StringStream& output)
{
	bool isFirst = true;
	u32 flagsAsInteger = (u32)flags;
	while(flagsAsInteger != 0)
	{
		const u32 flagIndex = Bitwise::LeastSignificantBit(flagsAsInteger);
		const GpuStageFlag flag = (GpuStageFlag)(1 << flagIndex);

		if(!isFirst)
			output << " | ";

		output << GetAccessStageName(flag);

		flagsAsInteger &= ~(1 << flagIndex);
		isFirst = false;
	}
}

void GpuBackendUtility::CutRange(const GpuTextureSubresourceRange& toCut, const GpuTextureSubresourceRange& cutWith, std::array<GpuTextureSubresourceRange, 5>& output, u32& numAreas)
{
	B3D_ASSERT(toCut.HasSingleAspect());
	B3D_ASSERT(toCut.AspectMask == cutWith.AspectMask);

	numAreas = 0;

	// Cut horizontally
	u32 numHorzCuts = 0;
	std::array<GpuTextureSubresourceRange, 3> horzCuts;
	CutHorizontalRange(toCut, cutWith, horzCuts.data(), numHorzCuts);

	// Cut vertically
	for(u32 i = 0; i < numHorzCuts; i++)
	{
		GpuTextureSubresourceRange& range = horzCuts[i];

		if(range.BaseArrayLayer >= cutWith.BaseArrayLayer &&
		   (range.BaseArrayLayer + range.ArrayLayerCount) <= (cutWith.BaseArrayLayer + cutWith.ArrayLayerCount))
		{
			u32 numVertCuts = 0;
			CutVerticalRange(range, cutWith, output.data() + numAreas, numVertCuts);

			numAreas += numVertCuts;
		}
		else
		{
			output[numAreas] = range;
			numAreas++;
		}
	}

	B3D_ASSERT(numAreas <= 5);
}

bool GpuBackendUtility::RangeOverlaps(const GpuTextureSubresourceRange& a, const GpuTextureSubresourceRange& b)
{
	if(!a.AspectMask.IsSetAny(b.AspectMask))
		return false;

	i32 aRight = a.BaseArrayLayer + (i32)a.ArrayLayerCount;
	i32 bRight = b.BaseArrayLayer + (i32)b.ArrayLayerCount;

	i32 aBottom = a.BaseMipLevel + (i32)a.MipLevelCount;
	i32 bBottom = b.BaseMipLevel + (i32)b.MipLevelCount;

	if((i32)a.BaseArrayLayer < bRight && aRight > (i32)b.BaseArrayLayer &&
	   (i32)a.BaseMipLevel < bBottom && aBottom > (i32)b.BaseMipLevel)
		return true;

	return false;
}

bool GpuBackendUtility::RangeEquals(const GpuTextureSubresourceRange& a, const GpuTextureSubresourceRange& b)
{
	return a.BaseArrayLayer == b.BaseArrayLayer && a.BaseMipLevel == b.BaseMipLevel &&
		a.ArrayLayerCount == b.ArrayLayerCount && a.MipLevelCount == b.MipLevelCount && a.AspectMask == b.AspectMask;
}

GpuTextureSubresourceRange GpuBackendUtility::GetBoundingRange(const GpuTextureSubresourceRange& a, const GpuTextureSubresourceRange& b)
{
	const u32 baseMipLevel = std::min(a.BaseMipLevel, b.BaseMipLevel);
	const u32 mipLevelEnd = std::max(a.BaseMipLevel + a.MipLevelCount, b.BaseMipLevel + b.MipLevelCount);
	const u32 baseArrayLayer = std::min(a.BaseArrayLayer, b.BaseArrayLayer);
	const u32 arrayLayerEnd = std::max(a.BaseArrayLayer + a.ArrayLayerCount, b.BaseArrayLayer + b.ArrayLayerCount);

	return GpuTextureSubresourceRange(baseMipLevel, mipLevelEnd - baseMipLevel, baseArrayLayer, arrayLayerEnd - baseArrayLayer, a.AspectMask | b.AspectMask);
}

bool GpuBackendUtility::RangeContains(const GpuTextureSubresourceRange& outer, const GpuTextureSubresourceRange& inner)
{
	return outer.AspectMask.IsSetAll(inner.AspectMask) &&
		outer.BaseMipLevel <= inner.BaseMipLevel && inner.BaseMipLevel + inner.MipLevelCount <= outer.BaseMipLevel + outer.MipLevelCount &&
		outer.BaseArrayLayer <= inner.BaseArrayLayer && inner.BaseArrayLayer + inner.ArrayLayerCount <= outer.BaseArrayLayer + outer.ArrayLayerCount;
}

GpuTextureAspectFlags GpuBackendUtility::GetFormatAspects(PixelFormat format)
{
	if(!PixelUtility::IsDepth(format))
		return GpuTextureAspectFlag::Color;

	// Only the combined depth-stencil formats carry a stencil aspect
	const bool hasStencil = format == PF_D24S8 || format == PF_D32_S8X24;
	return hasStencil ? (GpuTextureAspectFlag::Depth | GpuTextureAspectFlag::Stencil) : GpuTextureAspectFlags(GpuTextureAspectFlag::Depth);
}

GpuTextureSubresourceRange GpuBackendUtility::ClampRange(GpuTextureSubresourceRange range, const GpuTextureSubresourceRange& imageRange)
{
	if(range.ArrayLayerCount == ~0u)
		range.ArrayLayerCount = imageRange.ArrayLayerCount;

	if(range.MipLevelCount == ~0u)
		range.MipLevelCount = imageRange.MipLevelCount;

	range.AspectMask &= imageRange.AspectMask;
	return range;
}

GpuTextureSubresourceRange GpuBackendUtility::GetSurfaceRange(const GpuTextureSubresourceRange& imageRange, const TextureSurface& surface)
{
	const u32 remainingFaceCount = surface.Face < imageRange.ArrayLayerCount ? imageRange.ArrayLayerCount - surface.Face : 0;
	const u32 remainingMipLevelCount = surface.MipLevel < imageRange.MipLevelCount ? imageRange.MipLevelCount - surface.MipLevel : 0;

	GpuTextureSubresourceRange range;
	range.BaseArrayLayer = surface.Face;
	range.ArrayLayerCount = surface.FaceCount == 0 ? remainingFaceCount : Math::Min(surface.FaceCount, remainingFaceCount);
	range.BaseMipLevel = surface.MipLevel;
	range.MipLevelCount = surface.MipLevelCount == 0 ? remainingMipLevelCount : Math::Min(surface.MipLevelCount, remainingMipLevelCount);
	range.AspectMask = imageRange.AspectMask;
	return range;
}

const char* GpuBackendUtility::GetImageLayoutName(GpuImageLayout layout)
{
	switch(layout)
	{
	case GpuImageLayout::Undefined: return "Undefined";
	case GpuImageLayout::General: return "General";
	case GpuImageLayout::ColorAttachment: return "ColorAttachment";
	case GpuImageLayout::DepthStencilAttachment: return "DepthStencilAttachment";
	case GpuImageLayout::DepthStencilReadOnly: return "DepthStencilReadOnly";
	case GpuImageLayout::DepthReadOnlyStencilAttachment: return "DepthReadOnlyStencilAttachment";
	case GpuImageLayout::DepthAttachmentStencilReadOnly: return "DepthAttachmentStencilReadOnly";
	case GpuImageLayout::ShaderReadOnly: return "ShaderReadOnly";
	case GpuImageLayout::TransferSource: return "TransferSource";
	case GpuImageLayout::TransferDestination: return "TransferDestination";
	case GpuImageLayout::ResolveSource: return "ResolveSource";
	case GpuImageLayout::ResolveDestination: return "ResolveDestination";
	case GpuImageLayout::Present: return "Present";
	default: return "Unknown";
	}
}

void GpuBackendUtility::CopyToStagingBuffer(GpuCommandBuffer& commandBuffer, const TShared<render::Texture>& texture, u32 mipLevel, u32 arrayLayer, const TShared<render::GpuBuffer>& stagingBuffer)
{
	const GpuAccessState restingState = GetRestingState(*texture, commandBuffer.GetQueueType());
	const GpuAccessState transferRead(GpuStageFlag::Transfer, GpuAccessFlag::Read, GpuImageLayout::TransferSource);
	const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write);
	const GpuAccessState stagingRestingState = GetRestingState(*stagingBuffer, commandBuffer.GetQueueType());
	const GpuTextureSubresourceRange range(mipLevel, 1, arrayLayer, 1, GetFormatAspects(texture->GetProperties().Format));

	commandBuffer.IssueBarriers(GpuBarriers(
		{ GpuBufferBarrier(stagingBuffer, stagingRestingState, transferWrite) },
		{ GpuTextureBarrier(texture, restingState, transferRead, range) }));
	commandBuffer.CopyTextureToBuffer(texture, stagingBuffer, mipLevel, arrayLayer, 0);
	commandBuffer.IssueBarriers(GpuBarriers(
		{ GpuBufferBarrier(stagingBuffer, transferWrite, stagingRestingState) },
		{ GpuTextureBarrier(texture, transferRead, restingState, range) }));
}

void GpuBackendUtility::CopyToStagingBuffer(GpuCommandBuffer& commandBuffer, const TShared<render::GpuBuffer>& buffer, u32 offset, u32 length, const TShared<render::GpuBuffer>& stagingBuffer)
{
	const GpuAccessState restingState = GetRestingState(*buffer, commandBuffer.GetQueueType());
	const GpuAccessState transferRead(GpuStageFlag::Transfer, GpuAccessFlag::Read);
	const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write);
	const GpuAccessState stagingRestingState = GetRestingState(*stagingBuffer, commandBuffer.GetQueueType());

	commandBuffer.IssueBarriers(GpuBarriers({
		GpuBufferBarrier(buffer, restingState, transferRead),
		GpuBufferBarrier(stagingBuffer, stagingRestingState, transferWrite) }, {}));
	commandBuffer.CopyBufferToBuffer(buffer, stagingBuffer, offset, 0, length);
	commandBuffer.IssueBarriers(GpuBarriers({
		GpuBufferBarrier(buffer, transferRead, restingState),
		GpuBufferBarrier(stagingBuffer, transferWrite, stagingRestingState) }, {}));
}

void GpuBackendUtility::CopyToStagingBuffer(GpuCommandBuffer& commandBuffer, const TShared<render::RenderWindow>& window, const TShared<render::GpuBuffer>& stagingBuffer)
{
	const TShared<render::RenderTarget> target = window;
	const GpuAccessState restingState = GetRestingState(*window, RT_COLOR0, commandBuffer.GetQueueType());
	const GpuAccessState transferRead(GpuStageFlag::Transfer, GpuAccessFlag::Read, GpuImageLayout::TransferSource);
	const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write);
	const GpuAccessState stagingRestingState = GetRestingState(*stagingBuffer, commandBuffer.GetQueueType());

	commandBuffer.IssueBarriers(GpuBarriers(
		{ GpuBufferBarrier(stagingBuffer, stagingRestingState, transferWrite) }, {},
		{ GpuRenderTargetBarrier(target, RT_COLOR0, restingState, transferRead) }));
	commandBuffer.CopyWindowToBuffer(window, stagingBuffer);
	commandBuffer.IssueBarriers(GpuBarriers(
		{ GpuBufferBarrier(stagingBuffer, transferWrite, stagingRestingState) }, {},
		{ GpuRenderTargetBarrier(target, RT_COLOR0, transferRead, restingState) }));
}

GpuAccessState GpuBackendUtility::GetRestingState(const render::GpuBuffer& buffer, GpuQueueType queueType)
{
	// Only the host reads readback buffers, so the barrier that returns one to rest makes the GPU's writes visible to the host
	if(buffer.GetInformation().Type == GpuBufferType::StagingRead)
		return GpuAccessState(GetQueueStageFlags(queueType) & GpuStageFlag::Host, GpuAccessFlag::Read);

	// Only copies read upload buffers, and the host writes them before the submission that reads them
	if(buffer.GetInformation().Type == GpuBufferType::StagingWrite)
		return GpuAccessState(GetQueueStageFlags(queueType) & GpuStageFlag::Transfer, GpuAccessFlag::Read);

	const GpuStageFlags stages = GpuStageFlag::DrawIndirect | GpuStageFlag::VertexInputAttributes | GpuStageFlag::VertexInputIndices | GpuStageFlag::AllShader;
	return GpuAccessState(stages & GetQueueStageFlags(queueType), GpuAccessFlag::Read);
}

GpuAccessState GpuBackendUtility::GetRestingState(const render::Texture& texture, GpuQueueType queueType)
{
	const GpuStageFlags stages = GpuStageFlag::VertexShaderNonUniform | GpuStageFlag::FragmentShaderNonUniform | GpuStageFlag::ComputeShaderNonUniform;

	// Only render targets, depth-stencil targets and storage textures can have compression metadata. Whoever leaves their write state
	// resolves fast clears on the way back to rest, so sampling reads the contents directly.
	const TextureUsageFlags metadataUsage = TextureUsageFlag::RenderTarget | TextureUsageFlag::DepthStencil | TextureUsageFlag::AllowUnorderedAccessOnTheGPU;
	const GpuCompressionHint compression = texture.GetProperties().Usage.IsSetAny(metadataUsage) ? GpuCompressionHint::Compressed : GpuCompressionHint::Unknown;

	return GpuAccessState(stages & GetQueueStageFlags(queueType), GpuAccessFlag::Read, GpuImageLayout::ShaderReadOnly, compression);
}

GpuAccessState GpuBackendUtility::GetRestingState(const render::RenderTarget& target, RenderSurfaceMaskBits surface, GpuQueueType queueType)
{
	const bool isColor = ((u32)surface & (u32)RT_COLOR_ALL) != 0;
	if(!target.GetProperties().IsWindow)
	{
		const render::RenderTexture& renderTexture = static_cast<const render::RenderTexture&>(target);
		const render::RenderSurfaceInformation& surfaceInformation = isColor ? renderTexture.GetColorSurfaceInformation(Bitwise::LeastSignificantBit((u32)surface)) : renderTexture.GetDepthStencilSurfaceInformation();
		if(surfaceInformation.Texture != nullptr)
			return GetRestingState(*surfaceInformation.Texture, queueType);

		return GpuAccessState();
	}

	if(isColor)
		return GetWindowColorRestingState(queueType);

	const GpuStageFlags depthStencilStages = GpuStageFlag::EarlyFragmentTests | GpuStageFlag::LateFragmentTests;
	return GpuAccessState(depthStencilStages & GetQueueStageFlags(queueType), GpuAccessFlag::Read | GpuAccessFlag::Write, GpuImageLayout::DepthStencilAttachment);
}

GpuAccessState GpuBackendUtility::GetWindowColorRestingState(GpuQueueType queueType)
{
	return GpuAccessState(GpuStageFlags(GpuStageFlag::ColorAttachment) & GetQueueStageFlags(queueType), GpuAccessFlag::Read | GpuAccessFlag::Write, GpuImageLayout::ColorAttachment);
}

GpuImageLayout GpuBackendUtility::GetRenderPassAttachmentLayout(RenderSurfaceMaskBits surface, RenderSurfaceMask readOnlyMask)
{
	if(((u32)surface & (u32)RT_COLOR_ALL) != 0)
		return readOnlyMask.IsSet(surface) ? GpuImageLayout::General : GpuImageLayout::ColorAttachment;

	const bool depthReadOnly = readOnlyMask.IsSet(RT_DEPTH);
	const bool stencilReadOnly = readOnlyMask.IsSet(RT_STENCIL);
	if(depthReadOnly)
		return stencilReadOnly ? GpuImageLayout::DepthStencilReadOnly : GpuImageLayout::DepthReadOnlyStencilAttachment;

	return stencilReadOnly ? GpuImageLayout::DepthAttachmentStencilReadOnly : GpuImageLayout::DepthStencilAttachment;
}

const GpuRenderPassAttachmentUsage* GpuBackendUtility::FindRenderPassAttachment(TArrayView<const GpuRenderPassAttachmentUsage> attachments,
	IGpuImageResource* image, const GpuTextureSubresourceRange& range)
{
	for(const GpuRenderPassAttachmentUsage& attachment : attachments)
	{
		if(attachment.Image == image && RangeOverlaps(attachment.Range, range))
			return &attachment;
	}

	return nullptr;
}

GpuImageLayout GpuBackendUtility::ResolveShaderImageLayout(TArrayView<const GpuRenderPassAttachmentUsage> attachments, IGpuImageResource* image,
	const GpuTextureSubresourceRange& subresourceRange, GpuImageLayout requestedLayout)
{
	const GpuRenderPassAttachmentUsage* const renderPassAttachment = FindRenderPassAttachment(attachments, image, subresourceRange);
	if(renderPassAttachment != nullptr)
		return renderPassAttachment->Layout;

	return requestedLayout;
}

RenderSurfaceMask GpuBackendUtility::GetRenderPassClears(const RenderPassCreateInformation& createInformation, RenderTargetClearValues& clearValues)
{
	clearValues = createInformation.Target != nullptr ? createInformation.Target->GetClearValues() : RenderTargetClearValues();
	RenderSurfaceMask clearMask = createInformation.ClearMask;

#if B3D_BUILD_TYPE_DEVELOPMENT
	if(!gGpuPoisonUnloadedAttachments)
		return clearMask;

	const TShared<render::RenderTarget>& target = createInformation.Target;
	if(target == nullptr)
		return clearMask;

	const RenderSurfaceMask loadMask = createInformation.LoadMask;
	RenderSurfaceMask poisonMask = GetTargetSurfaces(*target) & ~loadMask & ~clearMask;

	// Backends load depth and stencil together
	if(loadMask.IsSetAny(RT_DEPTH_STENCIL))
		poisonMask &= ~RenderSurfaceMask(RT_DEPTH_STENCIL);

	// A read-only surface keeps its contents, so the pass must load it
	const RenderSurfaceMask unloadedReadOnlyMask = poisonMask & createInformation.ReadOnlyMask;
	if(unloadedReadOnlyMask != RT_NONE)
	{
		B3D_LOG(Error, LogRenderBackend, "Render pass surfaces {0} are read-only but not loaded. Read-only surfaces must be loaded.", (u32)unloadedReadOnlyMask);
		poisonMask &= ~unloadedReadOnlyMask;
	}

	SetPoisonClearValues(poisonMask, clearValues);
	clearMask |= poisonMask;
#endif

	return clearMask;
}

RenderSurfaceMask GpuBackendUtility::GetRenderPassDiscardClears([[maybe_unused]] const RenderPassCreateInformation& createInformation, RenderTargetClearValues& clearValues)
{
	clearValues = RenderTargetClearValues();

#if B3D_BUILD_TYPE_DEVELOPMENT
	if(!gGpuPoisonUnloadedAttachments || createInformation.Target == nullptr)
		return RT_NONE;

	const RenderSurfaceMask poisonMask = GetTargetSurfaces(*createInformation.Target) & ~createInformation.StoreMask & ~createInformation.ReadOnlyMask;
	SetPoisonClearValues(poisonMask, clearValues);
	return poisonMask;
#else
	return RT_NONE;
#endif
}
