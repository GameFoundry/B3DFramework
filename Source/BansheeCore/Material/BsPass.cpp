//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Material/BsPass.h"
#include "RenderAPI/BsRasterizerState.h"
#include "RenderAPI/BsBlendState.h"
#include "RenderAPI/BsDepthStencilState.h"
#include "Private/RTTI/BsPassRTTI.h"
#include "Material/BsMaterial.h"
#include "RenderAPI/BsGpuParams.h"
#include "RenderAPI/BsGpuProgram.h"
#include "RenderAPI/BsGpuPipelineState.h"

namespace bs
{
	/** Converts a sim thread pass descriptor to a core thread one. */
	void convertPassDesc(const PASS_DESC& input, ct::PASS_DESC& output)
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
	void convertPassDesc(const ct::PASS_DESC& input, ct::PIPELINE_STATE_DESC& output)
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

	Pass::Pass(const PASS_DESC& desc)
		:TPass(desc)
	{ }

	SPtr<ct::Pass> Pass::getCore() const
	{
		return std::static_pointer_cast<ct::Pass>(mCoreSpecific);
	}

	SPtr<ct::CoreObject> Pass::createCore() const
	{
		ct::PASS_DESC desc;
		convertPassDesc(mData, desc);

		ct::Pass* pass;
		if(mComputePipelineState != nullptr)
		{
			SPtr<ct::ComputePipelineState> corePipeline = mComputePipelineState->getCore();
			pass = new (bs_alloc<ct::Pass>()) ct::Pass(desc, corePipeline);
		}
		else
		{
			SPtr<ct::GraphicsPipelineState> corePipeline;
			if (mGraphicsPipelineState != nullptr)
				corePipeline = mGraphicsPipelineState->getCore();

			pass = new (bs_alloc<ct::Pass>()) ct::Pass(desc, corePipeline);
		}

		SPtr<ct::Pass> passPtr = bs_shared_ptr<ct::Pass>(pass);
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
		UINT32 size = sizeof(ct::PASS_DESC);

		UINT8* data = allocator->alloc(size);
		ct::PASS_DESC* passDesc = new (data) ct::PASS_DESC();
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

	namespace ct
	{
	Pass::Pass(const PASS_DESC& desc)
		:TPass(desc)
	{ }

	Pass::Pass(const PASS_DESC& desc, const SPtr<GraphicsPipelineState>& pipelineState)
		:TPass(desc)
	{
		mGraphicsPipelineState = pipelineState;
	}


	Pass::Pass(const PASS_DESC& desc, const SPtr<ComputePipelineState>& pipelineState)
		:TPass(desc)
	{
		mComputePipelineState = pipelineState;
	}

	void Pass::initialize()
	{
		if(hasComputeProgram())
		{
			if(mComputePipelineState == nullptr)
				mComputePipelineState = ComputePipelineState::create(getComputeProgram());
		}
		else
		{
			if (mGraphicsPipelineState == nullptr)
			{
				PIPELINE_STATE_DESC desc;
				convertPassDesc(mData, desc);

				mGraphicsPipelineState = GraphicsPipelineState::create(desc);
			}
		}

		CoreObject::initialize();
	}

	void Pass::syncToCore(const CoreSyncData& data)
	{
		UINT8* dataPtr = data.getBuffer();
		PASS_DESC* desc = (PASS_DESC*)dataPtr;

		mData = *desc;
		desc->~PASS_DESC();
	}

	SPtr<Pass> Pass::create(const PASS_DESC& desc)
	{
		Pass* newPass = new (bs_alloc<Pass>()) Pass(desc);
		SPtr<Pass> newPassPtr = bs_shared_ptr<Pass>(newPass);
		newPassPtr->_setThisPtr(newPassPtr);
		newPassPtr->initialize();

		return newPassPtr;
	}
	}
}