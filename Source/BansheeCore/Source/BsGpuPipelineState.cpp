//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGpuPipelineState.h"
#include "BsRasterizerState.h"
#include "BsBlendState.h"
#include "BsDepthStencilState.h"
#include "BsGpuProgram.h"
#include "BsGpuParamDesc.h"
#include "BsGpuPipelineParamInfo.h"
#include "BsRenderStateManager.h"

namespace bs
{
	/** Converts a sim thread pipeline state descriptor to a core thread one. */
	void convertPassDesc(const PIPELINE_STATE_DESC& input, PIPELINE_STATE_CORE_DESC& output)
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
	TGraphicsPipelineState<Core>::TGraphicsPipelineState()
	{ }

	template<bool Core>
	TGraphicsPipelineState<Core>::TGraphicsPipelineState(const StateDescType& data)
		:mData(data)
	{ }

	template class TGraphicsPipelineState < false > ;
	template class TGraphicsPipelineState < true >;

	GraphicsPipelineStateCore::GraphicsPipelineStateCore(const PIPELINE_STATE_CORE_DESC& desc, GpuDeviceFlags deviceMask)
		:TGraphicsPipelineState(desc)
	{
		GPU_PIPELINE_PARAMS_DESC paramsDesc;
		if (desc.vertexProgram != nullptr)
			paramsDesc.vertexParams = desc.vertexProgram->getParamDesc();

		if (desc.fragmentProgram != nullptr)
			paramsDesc.fragmentParams = desc.fragmentProgram->getParamDesc();

		if (desc.geometryProgram != nullptr)
			paramsDesc.geometryParams = desc.geometryProgram->getParamDesc();

		if (desc.hullProgram != nullptr)
			paramsDesc.hullParams = desc.hullProgram->getParamDesc();

		if (desc.domainProgram != nullptr)
			paramsDesc.domainParams = desc.domainProgram->getParamDesc();

		mParamInfo = GpuPipelineParamInfoCore::create(paramsDesc, deviceMask);
	}

	SPtr<GraphicsPipelineStateCore> GraphicsPipelineStateCore::create(const PIPELINE_STATE_CORE_DESC& desc, GpuDeviceFlags deviceMask)
	{
		return RenderStateCoreManager::instance().createGraphicsPipelineState(desc, deviceMask);
	}

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

	SPtr<GraphicsPipelineStateCore> GraphicsPipelineState::getCore() const
	{
		return std::static_pointer_cast<GraphicsPipelineStateCore>(mCoreSpecific);
	}

	SPtr<CoreObjectCore> GraphicsPipelineState::createCore() const
	{
		PIPELINE_STATE_CORE_DESC desc;
		convertPassDesc(mData, desc);

		return RenderStateCoreManager::instance()._createGraphicsPipelineState(desc);
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

	ComputePipelineStateCore::ComputePipelineStateCore(const SPtr<GpuProgramCore>& program, GpuDeviceFlags deviceMask)
		:TComputePipelineState(program)
	{
		GPU_PIPELINE_PARAMS_DESC paramsDesc;
		paramsDesc.computeParams = program->getParamDesc();

		mParamInfo = GpuPipelineParamInfoCore::create(paramsDesc, deviceMask);
	}

	SPtr<ComputePipelineStateCore> ComputePipelineStateCore::create(const SPtr<GpuProgramCore>& program, 
		GpuDeviceFlags deviceMask)
	{
		return RenderStateCoreManager::instance().createComputePipelineState(program, deviceMask);
	}

	ComputePipelineState::ComputePipelineState(const SPtr<GpuProgram>& program)
		:TComputePipelineState(program)
	{
		GPU_PIPELINE_PARAMS_DESC paramsDesc;
		program->blockUntilCoreInitialized();
		paramsDesc.computeParams = program->getParamDesc();

		mParamInfo = GpuPipelineParamInfo::create(paramsDesc);
	}

	SPtr<ComputePipelineStateCore> ComputePipelineState::getCore() const
	{
		return std::static_pointer_cast<ComputePipelineStateCore>(mCoreSpecific);
	}

	SPtr<CoreObjectCore> ComputePipelineState::createCore() const
	{
		return RenderStateCoreManager::instance()._createComputePipelineState(mProgram->getCore());
	}

	SPtr<ComputePipelineState> ComputePipelineState::create(const SPtr<GpuProgram>& program)
	{
		return RenderStateManager::instance().createComputePipelineState(program);
	}
}