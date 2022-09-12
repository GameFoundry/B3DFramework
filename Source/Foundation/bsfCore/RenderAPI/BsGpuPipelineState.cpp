//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsGpuPipelineState.h"
#include "RenderAPI/BsRasterizerState.h"
#include "RenderAPI/BsBlendState.h"
#include "RenderAPI/BsDepthStencilState.h"
#include "RenderAPI/BsGpuProgram.h"
#include "RenderAPI/BsGpuParamDesc.h"
#include "RenderAPI/BsGpuPipelineParamInfo.h"
#include "Managers/BsRenderStateManager.h"

namespace bs
{
	/** Converts a sim thread pipeline state descriptor to a core thread one. */
	void ConvertPassDesc(const PIPELINE_STATE_DESC& input, ct::PIPELINE_STATE_DESC& output)
	{
		output.blendState = input.blendState != nullptr ? input.blendState->GetCore() : nullptr;
		output.rasterizerState = input.rasterizerState != nullptr ? input.rasterizerState->GetCore() : nullptr;
		output.depthStencilState = input.depthStencilState != nullptr ? input.depthStencilState->GetCore() : nullptr;
		output.vertexProgram = input.vertexProgram != nullptr ? input.vertexProgram->GetCore() : nullptr;
		output.fragmentProgram = input.fragmentProgram != nullptr ? input.fragmentProgram->GetCore() : nullptr;
		output.geometryProgram = input.geometryProgram != nullptr ? input.geometryProgram->GetCore() : nullptr;
		output.hullProgram = input.hullProgram != nullptr ? input.hullProgram->GetCore() : nullptr;
		output.domainProgram = input.domainProgram != nullptr ? input.domainProgram->GetCore() : nullptr;
	}

	template<bool Core>
	TGraphicsPipelineState<Core>::TGraphicsPipelineState(const StateDescType& data)
		:mData(data)
	{ }

	template class TGraphicsPipelineState < false > ;
	template class TGraphicsPipelineState < true >;

	GraphicsPipelineState::GraphicsPipelineState(const PIPELINE_STATE_DESC& desc)
		:TGraphicsPipelineState(desc)
	{
		GPU_PIPELINE_PARAMS_DESC paramsDesc;
		if (desc.vertexProgram != nullptr)
		{
			desc.vertexProgram->BlockUntilCoreInitialized();
			paramsDesc.vertexParams = desc.vertexProgram->GetParamDesc();
		}

		if (desc.fragmentProgram != nullptr)
		{
			desc.fragmentProgram->BlockUntilCoreInitialized();
			paramsDesc.fragmentParams = desc.fragmentProgram->GetParamDesc();
		}

		if (desc.geometryProgram != nullptr)
		{
			desc.geometryProgram->BlockUntilCoreInitialized();
			paramsDesc.geometryParams = desc.geometryProgram->GetParamDesc();
		}

		if (desc.hullProgram != nullptr)
		{
			desc.hullProgram->BlockUntilCoreInitialized();
			paramsDesc.hullParams = desc.hullProgram->GetParamDesc();
		}
		
		if (desc.domainProgram != nullptr)
		{
			desc.domainProgram->BlockUntilCoreInitialized();
			paramsDesc.domainParams = desc.domainProgram->GetParamDesc();
		}

		mParamInfo = GpuPipelineParamInfo::create(paramsDesc);
	}

	SPtr<ct::GraphicsPipelineState> GraphicsPipelineState::GetCore() const
	{
		return std::static_pointer_cast<ct::GraphicsPipelineState>(mCoreSpecific);
	}

	SPtr<ct::CoreObject> GraphicsPipelineState::CreateCore() const
	{
		ct::PIPELINE_STATE_DESC desc;
		convertPassDesc(mData, desc);

		return ct::RenderStateManager::instance()._createGraphicsPipelineState(desc);
	}

	SPtr<GraphicsPipelineState> GraphicsPipelineState::Create(const PIPELINE_STATE_DESC& desc)
	{
		return RenderStateManager::Instance().CreateGraphicsPipelineState(desc);
	}

	template<bool Core>
	TComputePipelineState<Core>::TComputePipelineState()
	{ }

	template<bool Core>
	TComputePipelineState<Core>::TComputePipelineState(const GpuProgramType& program)
		:mProgram(program)
	{ }

	template class TComputePipelineState < false >;
	template class TComputePipelineState < true >;

	ComputePipelineState::ComputePipelineState(const SPtr<GpuProgram>& program)
		:TComputePipelineState(program)
	{
		GPU_PIPELINE_PARAMS_DESC paramsDesc;
		program->BlockUntilCoreInitialized();
		paramsDesc.computeParams = program->GetParamDesc();

		mParamInfo = GpuPipelineParamInfo::create(paramsDesc);
	}

	SPtr<ct::ComputePipelineState> ComputePipelineState::GetCore() const
	{
		return std::static_pointer_cast<ct::ComputePipelineState>(mCoreSpecific);
	}

	SPtr<ct::CoreObject> ComputePipelineState::CreateCore() const
	{
		return ct::RenderStateManager::instance()._createComputePipelineState(mProgram->GetCore());
	}

	SPtr<ComputePipelineState> ComputePipelineState::Create(const SPtr<GpuProgram>& program)
	{
		return RenderStateManager::Instance().CreateComputePipelineState(program);
	}

	namespace ct
	{
	GraphicsPipelineState::GraphicsPipelineState(const PIPELINE_STATE_DESC& desc, GpuDeviceFlags deviceMask)
		:TGraphicsPipelineState(desc), mDeviceMask(deviceMask)
	{ }

	void GraphicsPipelineState::Initialize()
	{
		GPU_PIPELINE_PARAMS_DESC paramsDesc;
		if (mData.vertexProgram != nullptr)
			paramsDesc.vertexParams = mData.vertexProgram->GetParamDesc();

		if (mData.fragmentProgram != nullptr)
			paramsDesc.fragmentParams = mData.fragmentProgram->GetParamDesc();

		if (mData.geometryProgram != nullptr)
			paramsDesc.geometryParams = mData.geometryProgram->GetParamDesc();

		if (mData.hullProgram != nullptr)
			paramsDesc.hullParams = mData.hullProgram->GetParamDesc();

		if (mData.domainProgram != nullptr)
			paramsDesc.domainParams = mData.domainProgram->GetParamDesc();

		mParamInfo = GpuPipelineParamInfo::create(paramsDesc, mDeviceMask);

		CoreObject::initialize();
	}

	SPtr<GraphicsPipelineState> GraphicsPipelineState::Create(const PIPELINE_STATE_DESC& desc, GpuDeviceFlags deviceMask)
	{
		return RenderStateManager::Instance().CreateGraphicsPipelineState(desc, deviceMask);
	}

	ComputePipelineState::ComputePipelineState(const SPtr<GpuProgram>& program, GpuDeviceFlags deviceMask)
		:TComputePipelineState(program), mDeviceMask(deviceMask)
	{ }

	void ComputePipelineState::Initialize()
	{
		GPU_PIPELINE_PARAMS_DESC paramsDesc;
		paramsDesc.computeParams = mProgram->GetParamDesc();

		mParamInfo = GpuPipelineParamInfo::create(paramsDesc, mDeviceMask);

		CoreObject::initialize();
	}

	SPtr<ComputePipelineState> ComputePipelineState::create(const SPtr<GpuProgram>& program,
		GpuDeviceFlags deviceMask)
	{
		return RenderStateManager::Instance().CreateComputePipelineState(program, deviceMask);
	}
	}
}
