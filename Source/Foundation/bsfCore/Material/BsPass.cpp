//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/BsPass.h"
#include "RenderAPI/BsRasterizerState.h"
#include "RenderAPI/BsBlendState.h"
#include "RenderAPI/BsDepthStencilState.h"
#include "Private/RTTI/BsPassRTTI.h"
#include "Material/BsMaterial.h"
#include "RenderAPI/BsGpuParams.h"
#include "RenderAPI/BsGpuProgram.h"
#include "RenderAPI/BsGpuPipelineState.h"
#include "CoreThread/BsCoreObjectSync.h"

namespace bs
{
	template<bool Core>
	TPass<Core>::TPass()
	{
		mData.stencilRefValue = 0;
	}

	template<bool Core>
	TPass<Core>::TPass(const PASS_DESC& data)
		:mData(data)
	{

	}

	template<bool Core>
	bool TPass<Core>::hasBlending() const
	{
		bool transparent = false;

		for (UINT32 i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; i++)
		{
			// Transparent if destination color is taken into account
			if (mData.blendStateDesc.renderTargetDesc[i].dstBlend != BF_ZERO ||
				mData.blendStateDesc.renderTargetDesc[i].srcBlend == BF_DEST_COLOR ||
				mData.blendStateDesc.renderTargetDesc[i].srcBlend == BF_INV_DEST_COLOR ||
				mData.blendStateDesc.renderTargetDesc[i].srcBlend == BF_DEST_ALPHA ||
				mData.blendStateDesc.renderTargetDesc[i].srcBlend == BF_INV_DEST_ALPHA)
			{
				transparent = true;
			}
		}

		return transparent;
	}

	template<bool Core>
	const GPU_PROGRAM_DESC& TPass<Core>::getProgramDesc(bs::GpuProgramType type) const
	{
		switch(type)
		{
		default:
		case GPT_VERTEX_PROGRAM:
			return mData.vertexProgramDesc;
		case GPT_FRAGMENT_PROGRAM:
			return mData.fragmentProgramDesc;
		case GPT_GEOMETRY_PROGRAM:
			return mData.geometryProgramDesc;
		case GPT_HULL_PROGRAM:
			return mData.hullProgramDesc;
		case GPT_DOMAIN_PROGRAM:
			return mData.domainProgramDesc;
		case GPT_COMPUTE_PROGRAM:
			return mData.computeProgramDesc;
		}
	}

	template<bool Core>
	void TPass<Core>::createPipelineState()
	{
		if (isCompute())
		{
			SPtr<GpuProgramType> program = GpuProgramType::create(mData.computeProgramDesc);
			mComputePipelineState = ComputePipelineStateType::create(program);
		}
		else
		{
			PipelineStateDescType desc;

			if(!mData.vertexProgramDesc.source.empty())
				desc.vertexProgram = GpuProgramType::create(mData.vertexProgramDesc);

			if(!mData.fragmentProgramDesc.source.empty())
				desc.fragmentProgram = GpuProgramType::create(mData.fragmentProgramDesc);

			if(!mData.geometryProgramDesc.source.empty())
				desc.geometryProgram = GpuProgramType::create(mData.geometryProgramDesc);

			if(!mData.hullProgramDesc.source.empty())
				desc.hullProgram = GpuProgramType::create(mData.hullProgramDesc);

			if(!mData.domainProgramDesc.source.empty())
				desc.domainProgram = GpuProgramType::create(mData.domainProgramDesc);

			desc.blendState = BlendStateType::create(mData.blendStateDesc);
			desc.rasterizerState = RasterizerStateType::create(mData.rasterizerStateDesc);
			desc.depthStencilState = DepthStencilStateType::create(mData.depthStencilStateDesc);

			mGraphicsPipelineState = GraphicsPipelineStateType::create(desc);
		}
	}

	template <bool Core>
	template <class P>
	void TPass<Core>::rttiEnumFields(P p)
	{
		p(mGraphicsPipelineState);
		p(mComputePipelineState);
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
		ct::Pass* pass = new (bs_alloc<ct::Pass>()) ct::Pass(mData);

		SPtr<ct::Pass> passPtr = bs_shared_ptr(pass);
		passPtr->_setThisPtr(passPtr);

		return passPtr;
	}

	void Pass::compile()
	{
		if(mComputePipelineState || mGraphicsPipelineState)
			return; // Already compiled

		// Note: It's possible (and quite likely) the pass has already been compiled on the core thread, so this will
		// unnecessarily recompile it. However syncing them in a clean way is not trivial hard and this method is currently
		// not being used much (at all) to warrant a complex solution. Something to keep in mind for later though.
		createPipelineState();

		markCoreDirty();
		CoreObject::syncToCore();
	}

	CoreSyncData Pass::syncToCore(FrameAlloc* allocator)
	{
		UINT32 size = csync_size(*this);
		UINT8* data = allocator->alloc(size);

		Bitstream stream(data, size);
		csync_write(*this, stream);

		return CoreSyncData(data, size);
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

	void Pass::compile()
	{
		if(mComputePipelineState || mGraphicsPipelineState)
			return; // Already compiled

		createPipelineState();
	}

	void Pass::syncToCore(const CoreSyncData& data)
	{
		Bitstream stream(data.getBuffer(), data.getBufferSize());
		csync_read(*this, stream);
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
