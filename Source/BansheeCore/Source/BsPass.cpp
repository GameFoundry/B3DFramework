//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsPass.h"
#include "BsRasterizerState.h"
#include "BsBlendState.h"
#include "BsDepthStencilState.h"
#include "BsPassRTTI.h"
#include "BsMaterial.h"
#include "BsGpuParams.h"
#include "BsFrameAlloc.h"
#include "BsGpuProgram.h"
#include "BsGpuPipelineState.h"

namespace bs
{
	/** Converts a sim thread pass descriptor to a core thread one. */
	void convertPassDesc(const PASS_DESC& input, PASS_DESC_CORE& output)
	{
		output.blendState = input.blendState != nullptr ? input.blendState->getCore() : nullptr;
		output.rasterizerState = input.rasterizerState != nullptr ? input.rasterizerState->getCore() : nullptr;
		output.depthStencilState = input.depthStencilState != nullptr ? input.depthStencilState->getCore() : nullptr;
		output.stencilRefValue = input.stencilRefValue;
		output.vertexProgram = input.vertexProgram != nullptr ? input.vertexProgram->getCore() : nullptr;
		output.fragmentProgram = input.fragmentProgram != nullptr ? input.fragmentProgram->getCore() : nullptr;
		output.geometryProgram = input.geometryProgram != nullptr ? input.geometryProgram->getCore() : nullptr;
		output.hullProgram = input.hullProgram != nullptr ? input.hullProgram->getCore() : nullptr;
		output.domainProgram = input.domainProgram != nullptr ? input.domainProgram->getCore() : nullptr;
		output.computeProgram = input.computeProgram != nullptr ? input.computeProgram->getCore() : nullptr;
	}

	/** Converts a sim thread pass descriptor to a pipeline state descriptor. */
	void convertPassDesc(const PASS_DESC& input, PIPELINE_STATE_DESC& output)
	{
		output.blendState = input.blendState ;
		output.rasterizerState = input.rasterizerState;
		output.depthStencilState = input.depthStencilState;
		output.vertexProgram = input.vertexProgram;
		output.fragmentProgram = input.fragmentProgram;
		output.geometryProgram = input.geometryProgram;
		output.hullProgram = input.hullProgram;
		output.domainProgram = input.domainProgram;
	}

	/** Converts a sim thread pass descriptor to a core thread pipeline state descriptor. */
	void convertPassDesc(const PASS_DESC_CORE& input, PIPELINE_STATE_CORE_DESC& output)
	{
		output.blendState = input.blendState;
		output.rasterizerState = input.rasterizerState;
		output.depthStencilState = input.depthStencilState;
		output.vertexProgram = input.vertexProgram;
		output.fragmentProgram = input.fragmentProgram;
		output.geometryProgram = input.geometryProgram;
		output.hullProgram = input.hullProgram;
		output.domainProgram = input.domainProgram;
	}

	template<bool Core>
	TPass<Core>::TPass()
	{
		mData.stencilRefValue = 0;
	}

	template<bool Core>
	TPass<Core>::TPass(const PassDescType& data)
		:mData(data)
	{

	}

	template<bool Core>
	bool TPass<Core>::hasBlending() const 
	{ 
		if (!mData.blendState)
			return false;

		bool transparent = false;

		const BlendProperties& bsProps = mData.blendState->getProperties();
		for (UINT32 i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; i++)
		{
			// Transparent if destination color is taken into account
			if (bsProps.getDstBlend(i) != BF_ZERO ||
				bsProps.getSrcBlend(i) == BF_DEST_COLOR ||
				bsProps.getSrcBlend(i) == BF_INV_DEST_COLOR ||
				bsProps.getSrcBlend(i) == BF_DEST_ALPHA ||
				bsProps.getSrcBlend(i) == BF_INV_DEST_ALPHA)
			{
				transparent = true;
			}
		}

		return transparent;
	}

	template class TPass < false > ;
	template class TPass < true >;

	PassCore::PassCore(const PASS_DESC_CORE& desc)
		:TPass(desc)
	{ }

	PassCore::PassCore(const PASS_DESC_CORE& desc, const SPtr<GraphicsPipelineStateCore>& pipelineState)
		:TPass(desc)
	{
		mGraphicsPipelineState = pipelineState;
	}


	PassCore::PassCore(const PASS_DESC_CORE& desc, const SPtr<ComputePipelineStateCore>& pipelineState)
		:TPass(desc)
	{
		mComputePipelineState = pipelineState;
	}

	void PassCore::initialize()
	{
		if(hasComputeProgram())
		{
			if(mComputePipelineState == nullptr)
				mComputePipelineState = ComputePipelineStateCore::create(getComputeProgram());
		}
		else
		{
			if (mGraphicsPipelineState == nullptr)
			{
				PIPELINE_STATE_CORE_DESC desc;
				convertPassDesc(mData, desc);

				mGraphicsPipelineState = GraphicsPipelineStateCore::create(desc);
			}
		}

		CoreObjectCore::initialize();
	}

	void PassCore::syncToCore(const CoreSyncData& data)
	{
		UINT8* dataPtr = data.getBuffer();
		PASS_DESC_CORE* desc = (PASS_DESC_CORE*)dataPtr;

		mData = *desc;
		desc->~PASS_DESC_CORE();
	}

	SPtr<PassCore> PassCore::create(const PASS_DESC_CORE& desc)
	{
		PassCore* newPass = new (bs_alloc<PassCore>()) PassCore(desc);
		SPtr<PassCore> newPassPtr = bs_shared_ptr<PassCore>(newPass);
		newPassPtr->_setThisPtr(newPassPtr);
		newPassPtr->initialize();

		return newPassPtr;
	}

	Pass::Pass(const PASS_DESC& desc)
		:TPass(desc)
	{ }

	SPtr<PassCore> Pass::getCore() const
	{
		return std::static_pointer_cast<PassCore>(mCoreSpecific);
	}

	SPtr<CoreObjectCore> Pass::createCore() const
	{
		PASS_DESC_CORE desc;
		convertPassDesc(mData, desc);

		PassCore* pass;
		if(mComputePipelineState != nullptr)
		{
			SPtr<ComputePipelineStateCore> corePipeline = mComputePipelineState->getCore();
			pass = new (bs_alloc<PassCore>()) PassCore(desc, corePipeline);
		}
		else
		{
			SPtr<GraphicsPipelineStateCore> corePipeline;
			if (mGraphicsPipelineState != nullptr)
				corePipeline = mGraphicsPipelineState->getCore();

			pass = new (bs_alloc<PassCore>()) PassCore(desc, corePipeline);
		}

		SPtr<PassCore> passPtr = bs_shared_ptr<PassCore>(pass);
		passPtr->_setThisPtr(passPtr);

		return passPtr;
	}

	void Pass::initialize()
	{
		// Create pipeline state
		if (hasComputeProgram())
		{
			mComputePipelineState = ComputePipelineState::create(getComputeProgram());
		}
		else
		{
			PIPELINE_STATE_DESC desc;
			convertPassDesc(mData, desc);

			mGraphicsPipelineState = GraphicsPipelineState::create(desc);
		}

		CoreObject::initialize();
	}

	CoreSyncData Pass::syncToCore(FrameAlloc* allocator)
	{
		UINT32 size = sizeof(PASS_DESC_CORE);

		UINT8* data = allocator->alloc(size);
		PASS_DESC_CORE* passDesc = new (data) PASS_DESC_CORE();
		convertPassDesc(mData, *passDesc);

		return CoreSyncData(data, size);
	}

	void Pass::getCoreDependencies(Vector<CoreObject*>& dependencies)
	{
		if (mData.blendState != nullptr)
			dependencies.push_back(mData.blendState.get());

		if (mData.rasterizerState != nullptr)
			dependencies.push_back(mData.rasterizerState.get());

		if (mData.depthStencilState != nullptr)
			dependencies.push_back(mData.depthStencilState.get());

		if (mData.vertexProgram != nullptr)
			dependencies.push_back(mData.vertexProgram.get());

		if (mData.fragmentProgram != nullptr)
			dependencies.push_back(mData.fragmentProgram.get());

		if (mData.geometryProgram != nullptr)
			dependencies.push_back(mData.geometryProgram.get());

		if (mData.hullProgram != nullptr)
			dependencies.push_back(mData.hullProgram.get());

		if (mData.domainProgram != nullptr)
			dependencies.push_back(mData.domainProgram.get());

		if (mData.computeProgram != nullptr)
			dependencies.push_back(mData.computeProgram.get());
	}

	SPtr<Pass> Pass::create(const PASS_DESC& desc)
	{
		Pass* newPass = new (bs_alloc<Pass>()) Pass(desc);
		SPtr<Pass> newPassPtr = bs_core_ptr<Pass>(newPass);
		newPassPtr->_setThisPtr(newPassPtr);
		newPassPtr->initialize();

		return newPassPtr;
	}

	SPtr<Pass> Pass::createEmpty()
	{
		Pass* newPass = new (bs_alloc<Pass>()) Pass();
		SPtr<Pass> newPassPtr = bs_core_ptr<Pass>(newPass);
		newPassPtr->_setThisPtr(newPassPtr);

		return newPassPtr;
	}

	RTTITypeBase* Pass::getRTTIStatic()
	{
		return PassRTTI::instance();
	}

	RTTITypeBase* Pass::getRTTI() const
	{
		return Pass::getRTTIStatic();
	}
}