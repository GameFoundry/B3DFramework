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
	void convertPassDesc(const PIPELINE_STATE_DESC& input, ct::PIPELINE_STATE_DESC& output)
	{
		output.blendState = input.blendState != nullptr ? input.blendState->getCore() : nullptr;
		output.rasterizerState = input.rasterizerState != nullptr ? input.rasterizerState->getCore() : nullptr;
		output.depthStencilState = input.depthStencilState != nullptr ? input.depthStencilState->getCore() : nullptr;
		output.vertexProgram = input.vertexProgram != nullptr ? input.vertexProgram->getCore() : nullptr;
		output.fragmentProgram = input.fragmentProgram != nullptr ? input.fragmentProgram->getCore() : nullptr;
		output.geometryProgram = input.geometryProgram != nullptr ? input.geometryProgram->getCore() : nullptr;
		output.hullProgram = input.hullProgram != nullptr ? input.hullProgram->getCore() : nullptr;
		output.domainProgram = input.domainProgram != nullptr ? input.domainProgram->getCore() : nullptr;
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
			desc.vertexProgram->blockUntilCoreInitialized();
			paramsDesc.vertexParams = desc.vertexProgram->getParamDesc();
		}

		if (desc.fragmentProgram != nullptr)
		{
			desc.fragmentProgram->blockUntilCoreInitialized();
			paramsDesc.fragmentParams = desc.fragmentProgram->getParamDesc();
		}

		if (desc.geometryProgram != nullptr)
		{
			desc.geometryProgram->blockUntilCoreInitialized();
			paramsDesc.geometryParams = desc.geometryProgram->getParamDesc();
		}

		if (desc.hullProgram != nullptr)
		{
			desc.hullProgram->blockUntilCoreInitialized();
			paramsDesc.hullParams = desc.hullProgram->getParamDesc();
		}
		
		if (desc.domainProgram != nullptr)
		{
			desc.domainProgram->blockUntilCoreInitialized();
			paramsDesc.domainParams = desc.domainProgram->getParamDesc();
		}

		mParamInfo = GpuPipelineParamInfo::create(paramsDesc);
	}

	SPtr<ct::GraphicsPipelineState> GraphicsPipelineState::getCore() const
	{
		return std::static_pointer_cast<ct::GraphicsPipelineState>(mCoreSpecific);
	}

	SPtr<ct::CoreObject> GraphicsPipelineState::createCore() const
	{
		ct::PIPELINE_STATE_DESC desc;
		convertPassDesc(mData, desc);

		return ct::RenderStateManager::instance()._createGraphicsPipelineState(desc);
	}

	SPtr<GraphicsPipelineState> GraphicsPipelineState::create(const PIPELINE_STATE_DESC& desc)
	{
		return RenderStateManager::instance().createGraphicsPipelineState(desc);
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
		program->blockUntilCoreInitialized();
		paramsDesc.computeParams = program->getParamDesc();

		mParamInfo = GpuPipelineParamInfo::create(paramsDesc);
	}

	SPtr<ct::ComputePipelineState> ComputePipelineState::getCore() const
	{
		return std::static_pointer_cast<ct::ComputePipelineState>(mCoreSpecific);
	}

	SPtr<ct::CoreObject> ComputePipelineState::createCore() const
	{
		return ct::RenderStateManager::instance()._createComputePipelineState(mProgram->getCore());
	}

	SPtr<ComputePipelineState> ComputePipelineState::create(const SPtr<GpuProgram>& program)
	{
		return RenderStateManager::instance().createComputePipelineState(program);
	}

	namespace ct
	{
	GraphicsPipelineState::GraphicsPipelineState(const PIPELINE_STATE_DESC& desc, GpuDeviceFlags deviceMask)
		:TGraphicsPipelineState(desc), mDeviceMask(deviceMask)
	{ }

	void GraphicsPipelineState::initialize()
	{
		GPU_PIPELINE_PARAMS_DESC paramsDesc;
		if (mData.vertexProgram != nullptr)
			paramsDesc.vertexParams = mData.vertexProgram->getParamDesc();

		if (mData.fragmentProgram != nullptr)
			paramsDesc.fragmentParams = mData.fragmentProgram->getParamDesc();

		if (mData.geometryProgram != nullptr)
			paramsDesc.geometryParams = mData.geometryProgram->getParamDesc();

		if (mData.hullProgram != nullptr)
			paramsDesc.hullParams = mData.hullProgram->getParamDesc();

		if (mData.domainProgram != nullptr)
			paramsDesc.domainParams = mData.domainProgram->getParamDesc();

		mParamInfo = GpuPipelineParamInfo::create(paramsDesc, mDeviceMask);

		CoreObject::initialize();
	}

	SPtr<GraphicsPipelineState> GraphicsPipelineState::create(const PIPELINE_STATE_DESC& desc, GpuDeviceFlags deviceMask)
	{
		return RenderStateManager::instance().createGraphicsPipelineState(desc, deviceMask);
	}

	ComputePipelineState::ComputePipelineState(const SPtr<GpuProgram>& program, GpuDeviceFlags deviceMask)
		:TComputePipelineState(program), mDeviceMask(deviceMask)
	{ }

	void ComputePipelineState::initialize()
	{
		GPU_PIPELINE_PARAMS_DESC paramsDesc;
		paramsDesc.computeParams = mProgram->getParamDesc();

		mParamInfo = GpuPipelineParamInfo::create(paramsDesc, mDeviceMask);

		CoreObject::initialize();
	}

	SPtr<ComputePipelineState> ComputePipelineState::create(const SPtr<GpuProgram>& program, 
		GpuDeviceFlags deviceMask)
	{
		return RenderStateManager::instance().createComputePipelineState(program, deviceMask);
	}
	}
}