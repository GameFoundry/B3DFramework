//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGpuPipelineState.h"
#include "BsRasterizerState.h"
#include "BsBlendState.h"
#include "BsDepthStencilState.h"
#include "BsGpuProgram.h"
#include "BsRenderStateManager.h"

namespace BansheeEngine
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
	TGpuPipelineState<Core>::TGpuPipelineState()
	{ }

	template<bool Core>
	TGpuPipelineState<Core>::TGpuPipelineState(const StateDescType& data)
		:mData(data)
	{ }

	template class TGpuPipelineState < false > ;
	template class TGpuPipelineState < true >;

	GpuPipelineStateCore::GpuPipelineStateCore(const PIPELINE_STATE_CORE_DESC& desc, GpuDeviceFlags deviceMask)
		:TGpuPipelineState(desc)
	{ }

	SPtr<GpuPipelineStateCore> GpuPipelineStateCore::create(const PIPELINE_STATE_CORE_DESC& desc, GpuDeviceFlags deviceMask)
	{
		return RenderStateCoreManager::instance().createPipelineState(desc, deviceMask);
	}

	GpuPipelineState::GpuPipelineState(const PIPELINE_STATE_DESC& desc)
		:TGpuPipelineState(desc)
	{ }

	SPtr<GpuPipelineStateCore> GpuPipelineState::getCore() const
	{
		return std::static_pointer_cast<GpuPipelineStateCore>(mCoreSpecific);
	}

	SPtr<CoreObjectCore> GpuPipelineState::createCore() const
	{
		PIPELINE_STATE_CORE_DESC desc;
		convertPassDesc(mData, desc);

		return RenderStateCoreManager::instance()._createPipelineState(desc);
	}

	SPtr<GpuPipelineState> GpuPipelineState::create(const PIPELINE_STATE_DESC& desc)
	{
		return RenderStateManager::instance().createPipelineState(desc);
	}
}