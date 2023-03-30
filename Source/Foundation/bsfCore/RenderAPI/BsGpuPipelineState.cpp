//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsGpuPipelineState.h"

#include "BsCoreApplication.h"
#include "BsGpuBackend.h"
#include "BsGpuDevice.h"
#include "RenderAPI/BsRasterizerState.h"
#include "RenderAPI/BsBlendState.h"
#include "RenderAPI/BsDepthStencilState.h"
#include "RenderAPI/BsGpuProgram.h"
#include "RenderAPI/BsGpuProgramParameterDescription.h"
#include "RenderAPI/BsGpuPipelineParameterLayout.h"
#include "Managers/BsRenderStateManager.h"

using namespace bs;

/** Converts a sim thread pipeline state descriptor to a core thread one. */
void ConvertPassDesc(const PIPELINE_STATE_DESC& input, ct::PIPELINE_STATE_DESC& output)
{
	output.BlendState = input.BlendState != nullptr ? input.BlendState->GetCore() : nullptr;
	output.RasterizerState = input.RasterizerState != nullptr ? input.RasterizerState->GetCore() : nullptr;
	output.DepthStencilState = input.DepthStencilState != nullptr ? input.DepthStencilState->GetCore() : nullptr;
	output.VertexProgram = input.VertexProgram != nullptr ? input.VertexProgram->GetCore() : nullptr;
	output.FragmentProgram = input.FragmentProgram != nullptr ? input.FragmentProgram->GetCore() : nullptr;
	output.GeometryProgram = input.GeometryProgram != nullptr ? input.GeometryProgram->GetCore() : nullptr;
	output.HullProgram = input.HullProgram != nullptr ? input.HullProgram->GetCore() : nullptr;
	output.DomainProgram = input.DomainProgram != nullptr ? input.DomainProgram->GetCore() : nullptr;
}

template <bool Core>
TGraphicsPipelineState<Core>::TGraphicsPipelineState(const StateDescType& data)
	: mData(data)
{}

template class TGraphicsPipelineState<false>;
template class TGraphicsPipelineState<true>;

GraphicsPipelineState::GraphicsPipelineState(const PIPELINE_STATE_DESC& desc)
	: TGraphicsPipelineState(desc)
{
	GpuPipelineParameterLayoutInformation paramsDesc;
	if(desc.VertexProgram != nullptr)
	{
		desc.VertexProgram->BlockUntilCoreInitialized();
		paramsDesc.Vertex = desc.VertexProgram->GetParameterDescription();
	}

	if(desc.FragmentProgram != nullptr)
	{
		desc.FragmentProgram->BlockUntilCoreInitialized();
		paramsDesc.Fragment = desc.FragmentProgram->GetParameterDescription();
	}

	if(desc.GeometryProgram != nullptr)
	{
		desc.GeometryProgram->BlockUntilCoreInitialized();
		paramsDesc.Geometry = desc.GeometryProgram->GetParameterDescription();
	}

	if(desc.HullProgram != nullptr)
	{
		desc.HullProgram->BlockUntilCoreInitialized();
		paramsDesc.Hull = desc.HullProgram->GetParameterDescription();
	}

	if(desc.DomainProgram != nullptr)
	{
		desc.DomainProgram->BlockUntilCoreInitialized();
		paramsDesc.Domain = desc.DomainProgram->GetParameterDescription();
	}

	mParameterLayout = GpuPipelineParameterLayout::Create(paramsDesc);
}

SPtr<ct::GraphicsPipelineState> GraphicsPipelineState::GetCore() const
{
	return std::static_pointer_cast<ct::GraphicsPipelineState>(mCoreSpecific);
}

SPtr<ct::CoreObject> GraphicsPipelineState::CreateCore() const
{
	ct::PIPELINE_STATE_DESC desc;
	ConvertPassDesc(mData, desc);

	return ct::RenderStateManager::Instance().CreateGraphicsPipelineStateInternal(desc);
}

SPtr<GraphicsPipelineState> GraphicsPipelineState::Create(const PIPELINE_STATE_DESC& desc)
{
	return RenderStateManager::Instance().CreateGraphicsPipelineState(desc);
}

template <bool Core>
TComputePipelineState<Core>::TComputePipelineState()
{}

template <bool Core>
TComputePipelineState<Core>::TComputePipelineState(const GpuProgramType& program)
	: mProgram(program)
{}

template class TComputePipelineState<false>;
template class TComputePipelineState<true>;

ComputePipelineState::ComputePipelineState(const SPtr<GpuProgram>& program)
	: TComputePipelineState(program)
{
	GpuPipelineParameterLayoutInformation paramsDesc;
	program->BlockUntilCoreInitialized();
	paramsDesc.Compute = program->GetParameterDescription();

	mParameterLayout = GpuPipelineParameterLayout::Create(paramsDesc);
}

SPtr<ct::ComputePipelineState> ComputePipelineState::GetCore() const
{
	return std::static_pointer_cast<ct::ComputePipelineState>(mCoreSpecific);
}

SPtr<ct::CoreObject> ComputePipelineState::CreateCore() const
{
	return ct::RenderStateManager::Instance().CreateComputePipelineStateInternal(mProgram->GetCore());
}

SPtr<ComputePipelineState> ComputePipelineState::Create(const SPtr<GpuProgram>& program)
{
	return RenderStateManager::Instance().CreateComputePipelineState(program);
}

namespace bs { namespace ct
{
GraphicsPipelineState::GraphicsPipelineState(const PIPELINE_STATE_DESC& desc, GpuDeviceFlags deviceMask)
	: TGraphicsPipelineState(desc), mDeviceMask(deviceMask)
{}

void GraphicsPipelineState::Initialize()
{
	GpuPipelineParameterLayoutInformation parameterLayoutCreateInformation;
	if(mData.VertexProgram != nullptr)
		parameterLayoutCreateInformation.Vertex = mData.VertexProgram->GetParameterDescription();

	if(mData.FragmentProgram != nullptr)
		parameterLayoutCreateInformation.Fragment = mData.FragmentProgram->GetParameterDescription();

	if(mData.GeometryProgram != nullptr)
		parameterLayoutCreateInformation.Geometry = mData.GeometryProgram->GetParameterDescription();

	if(mData.HullProgram != nullptr)
		parameterLayoutCreateInformation.Hull = mData.HullProgram->GetParameterDescription();

	if(mData.DomainProgram != nullptr)
		parameterLayoutCreateInformation.Domain = mData.DomainProgram->GetParameterDescription();

	B3D_ENSURE(mDeviceMask == GDF_PRIMARY || mDeviceMask == GDF_DEFAULT);
	const SPtr<GpuDevice>& gpuDevice = GpuBackend::Instance().GetDevice((mDeviceMask == GDF_PRIMARY || mDeviceMask == GDF_DEFAULT) ? 0 : ~0u);
	mParameterLayout = gpuDevice->CreateGpuPipelineParameterLayout(parameterLayoutCreateInformation);

	CoreObject::Initialize();
}

SPtr<GraphicsPipelineState> GraphicsPipelineState::Create(const PIPELINE_STATE_DESC& desc, GpuDeviceFlags deviceMask)
{
	return RenderStateManager::Instance().CreateGraphicsPipelineState(desc, deviceMask);
}

ComputePipelineState::ComputePipelineState(const SPtr<GpuProgram>& program, GpuDeviceFlags deviceMask)
	: TComputePipelineState(program), mDeviceMask(deviceMask)
{}

void ComputePipelineState::Initialize()
{
	GpuPipelineParameterLayoutInformation parameterLayoutCreateInformation;
	parameterLayoutCreateInformation.Compute = mProgram->GetParameterDescription();

	B3D_ENSURE(mDeviceMask == GDF_PRIMARY || mDeviceMask == GDF_DEFAULT);
	const SPtr<GpuDevice>& gpuDevice = GpuBackend::Instance().GetDevice((mDeviceMask == GDF_PRIMARY || mDeviceMask == GDF_DEFAULT) ? 0 : ~0u);
	mParameterLayout = gpuDevice->CreateGpuPipelineParameterLayout(parameterLayoutCreateInformation);

	CoreObject::Initialize();
}

SPtr<ComputePipelineState> ComputePipelineState::Create(const SPtr<GpuProgram>& program, GpuDeviceFlags deviceMask)
{
	return RenderStateManager::Instance().CreateComputePipelineState(program, deviceMask);
}
}}
