//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuBackendUtility.h"

#include "Utility/B3DBitwise.h"

using namespace b3d;
using namespace b3d::render;

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
