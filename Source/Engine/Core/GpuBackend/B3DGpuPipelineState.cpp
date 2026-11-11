//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuPipelineState.h"

#include "B3DGpuBackend.h"
#include "B3DGpuDevice.h"
#include "GpuBackend/B3DGpuProgram.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"
#include "GpuBackend/B3DGpuPipelineParameterLayout.h"

using namespace b3d;

bool RenderTargetBlendStateInformation::operator==(const RenderTargetBlendStateInformation& rhs) const
{
	return BlendEnable == rhs.BlendEnable &&
		ColorSourceFactor == rhs.ColorSourceFactor &&
		ColorDestinationFactor == rhs.ColorDestinationFactor &&
		ColorBlendOperation == rhs.ColorBlendOperation &&
		AlphaSourceFactor == rhs.AlphaSourceFactor &&
		AlphaDestinationFactor == rhs.AlphaDestinationFactor &&
		AlphaBlendOperation == rhs.AlphaBlendOperation &&
		RenderTargetWriteMask == rhs.RenderTargetWriteMask;
}

bool BlendStateInformation::operator==(const BlendStateInformation& rhs) const
{
	bool equals = EnableAlphaToCoverage == rhs.EnableAlphaToCoverage &&
		EnableIndependantBlend == rhs.EnableIndependantBlend;

	if(equals)
	{
		for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
		{
			equals &= RenderTargets[i] == rhs.RenderTargets[i];
		}
	}

	return equals;
}

u64 BlendStateInformation::GenerateHash(const BlendStateInformation& value)
{
	size_t hash = 0;
	B3DCombineHash(hash, value.EnableAlphaToCoverage);
	B3DCombineHash(hash, value.EnableIndependantBlend);

	for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
	{
		B3DCombineHash(hash, value.RenderTargets[i].BlendEnable);
		B3DCombineHash(hash, (u32)value.RenderTargets[i].ColorSourceFactor);
		B3DCombineHash(hash, (u32)value.RenderTargets[i].ColorDestinationFactor);
		B3DCombineHash(hash, (u32)value.RenderTargets[i].ColorBlendOperation);
		B3DCombineHash(hash, (u32)value.RenderTargets[i].AlphaSourceFactor);
		B3DCombineHash(hash, (u32)value.RenderTargets[i].AlphaDestinationFactor);
		B3DCombineHash(hash, (u32)value.RenderTargets[i].AlphaBlendOperation);
		B3DCombineHash(hash, value.RenderTargets[i].RenderTargetWriteMask);
	}

	return (u64)hash;
}

bool RasterizerStateInformation::operator==(const RasterizerStateInformation& rhs) const
{
	return PolygonMode == rhs.PolygonMode &&
		CullMode == rhs.CullMode &&
		DepthBias == rhs.DepthBias &&
		DepthBiasClamp == rhs.DepthBiasClamp &&
		SlopeScaledDepthBias == rhs.SlopeScaledDepthBias &&
		DepthClipEnable == rhs.DepthClipEnable &&
		ScissorEnable == rhs.ScissorEnable &&
		MultisampleEnable == rhs.MultisampleEnable &&
		AntialiasedLineEnable == rhs.AntialiasedLineEnable;
}

u64 RasterizerStateInformation::GenerateHash(const RasterizerStateInformation& value)
{
	size_t hash = 0;
	B3DCombineHash(hash, (u32)value.PolygonMode);
	B3DCombineHash(hash, (u32)value.CullMode);
	B3DCombineHash(hash, value.DepthBias);
	B3DCombineHash(hash, value.DepthBiasClamp);
	B3DCombineHash(hash, value.SlopeScaledDepthBias);
	B3DCombineHash(hash, value.DepthClipEnable);
	B3DCombineHash(hash, value.ScissorEnable);
	B3DCombineHash(hash, value.MultisampleEnable);
	B3DCombineHash(hash, value.AntialiasedLineEnable);

	return (u64)hash;
}

bool DepthStencilStateInformation::operator==(const DepthStencilStateInformation& rhs) const
{
	return DepthReadEnable == rhs.DepthReadEnable &&
		DepthWriteEnable == rhs.DepthWriteEnable &&
		DepthComparisonFunc == rhs.DepthComparisonFunc &&
		StencilEnable == rhs.StencilEnable &&
		StencilReadMask == rhs.StencilReadMask &&
		StencilWriteMask == rhs.StencilWriteMask &&
		FrontStencilFailOp == rhs.FrontStencilFailOp &&
		FrontStencilZFailOp == rhs.FrontStencilZFailOp &&
		FrontStencilPassOp == rhs.FrontStencilPassOp &&
		FrontStencilComparisonFunc == rhs.FrontStencilComparisonFunc &&
		BackStencilFailOp == rhs.BackStencilFailOp &&
		BackStencilZFailOp == rhs.BackStencilZFailOp &&
		BackStencilPassOp == rhs.BackStencilPassOp &&
		BackStencilComparisonFunc == rhs.BackStencilComparisonFunc;
}

u64 DepthStencilStateInformation::GenerateHash(const DepthStencilStateInformation& value)
{
	size_t hash = 0;
	B3DCombineHash(hash, value.DepthReadEnable);
	B3DCombineHash(hash, value.DepthWriteEnable);
	B3DCombineHash(hash, (u32)value.DepthComparisonFunc);
	B3DCombineHash(hash, value.StencilEnable);
	B3DCombineHash(hash, value.StencilReadMask);
	B3DCombineHash(hash, value.StencilWriteMask);
	B3DCombineHash(hash, (u32)value.FrontStencilFailOp);
	B3DCombineHash(hash, (u32)value.FrontStencilZFailOp);
	B3DCombineHash(hash, (u32)value.FrontStencilPassOp);
	B3DCombineHash(hash, (u32)value.FrontStencilComparisonFunc);
	B3DCombineHash(hash, (u32)value.BackStencilFailOp);
	B3DCombineHash(hash, (u32)value.BackStencilZFailOp);
	B3DCombineHash(hash, (u32)value.BackStencilPassOp);
	B3DCombineHash(hash, (u32)value.BackStencilComparisonFunc);

	return (u64)hash;
}

GpuGraphicsPipelineState::GpuGraphicsPipelineState(GpuDevice& gpuDevice, const GpuGraphicsPipelineStateCreateInformation& createInformation)
		: mGpuDevice(gpuDevice), mData(createInformation)
	{}

namespace
{
	/**
	 * Adds @p program to the programs a pipeline's parameter layout is built from, as a program of type @p type: its parameter
	 * description and, when its bytecode carries one, its reflected resource-table layout (consumed by backends that pack
	 * descriptors at compiler-chosen offsets). Does nothing if @p program is null.
	 */
	void AddParameterLayoutProgram(GpuPipelineParameterLayoutInformation& information, GpuProgramType type, const TShared<GpuProgram>& program)
	{
		if(program == nullptr)
			return;

		const TShared<GpuProgramBytecode> bytecode = program->GetBytecode();
		information.Programs.push_back(GpuPipelineParameterLayoutProgram(type, program->GetParameterDescription(), bytecode != nullptr ? bytecode->ResourceTableLayout : nullptr));
	}
} // namespace

void GpuGraphicsPipelineState::Initialize()
{
	GpuPipelineParameterLayoutInformation parameterLayoutCreateInformation;
	AddParameterLayoutProgram(parameterLayoutCreateInformation, GPT_VERTEX_PROGRAM, mData.VertexProgram);
	AddParameterLayoutProgram(parameterLayoutCreateInformation, GPT_FRAGMENT_PROGRAM, mData.FragmentProgram);
	AddParameterLayoutProgram(parameterLayoutCreateInformation, GPT_GEOMETRY_PROGRAM, mData.GeometryProgram);
	AddParameterLayoutProgram(parameterLayoutCreateInformation, GPT_DOMAIN_PROGRAM, mData.DomainProgram);
	AddParameterLayoutProgram(parameterLayoutCreateInformation, GPT_HULL_PROGRAM, mData.HullProgram);

	mParameterLayout = mGpuDevice.CreateGpuPipelineParameterLayout(parameterLayoutCreateInformation);
}

GpuComputePipelineState::GpuComputePipelineState(GpuDevice& gpuDevice, const GpuComputePipelineStateCreateInformation& createInformation)
	: mGpuDevice(gpuDevice), mData(createInformation)
{}

void GpuComputePipelineState::Initialize()
{
	GpuPipelineParameterLayoutInformation parameterLayoutCreateInformation;
	AddParameterLayoutProgram(parameterLayoutCreateInformation, GPT_COMPUTE_PROGRAM, mData.Program);

	mParameterLayout = mGpuDevice.CreateGpuPipelineParameterLayout(parameterLayoutCreateInformation);
}

