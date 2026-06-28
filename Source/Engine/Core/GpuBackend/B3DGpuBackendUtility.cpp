//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuBackendUtility.h"

#include "Utility/B3DBitwise.h"
#include "Math/B3DMath.h"

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
}

GpuStageFlags GpuBackendUtility::GetStageFlags(GpuResourceUseFlags usage)
{
	GpuStageFlags accessStageFlags;

	if(usage.IsSet(GpuResourceUseFlag::ShaderAccess))
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

	if(usage.IsSet(GpuResourceUseFlag::Host))
		accessStageFlags |= GpuStageFlag::Host;

	if(usage.IsSet(GpuResourceUseFlag::ColorAttachment))
		accessStageFlags |= GpuStageFlag::ColorAttachment;

	if(usage.IsSet(GpuResourceUseFlag::DepthStencilAttachment))
		accessStageFlags |= GpuStageFlag::EarlyFragmentTests | GpuStageFlag::LateFragmentTests;

	return accessStageFlags;
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
	i32 aRight = a.BaseArrayLayer + (i32)a.ArrayLayerCount;
	i32 bRight = b.BaseArrayLayer + (i32)b.ArrayLayerCount;

	i32 aBottom = a.BaseMipLevel + (i32)a.MipLevelCount;
	i32 bBottom = b.BaseMipLevel + (i32)b.MipLevelCount;

	if((i32)a.BaseArrayLayer < bRight && aRight > (i32)b.BaseArrayLayer &&
	   (i32)a.BaseMipLevel < bBottom && aBottom > (i32)b.BaseMipLevel)
		return true;

	return false;
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
	case GpuImageLayout::Present: return "Present";
	default: return "Unknown";
	}
}
