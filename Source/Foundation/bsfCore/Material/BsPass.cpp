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

using namespace bs;

template <bool Core>
TPass<Core>::TPass()
{
	mData.StencilRefValue = 0;
}

template <bool Core>
TPass<Core>::TPass(const PASS_DESC& data)
	: mData(data)
{
}

template <bool Core>
bool TPass<Core>::HasBlending() const
{
	bool transparent = false;

	for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
	{
		// Transparent if destination color is taken into account
		if(mData.BlendStateDesc.RenderTargetDesc[i].DstBlend != BF_ZERO ||
		   mData.BlendStateDesc.RenderTargetDesc[i].SrcBlend == BF_DEST_COLOR ||
		   mData.BlendStateDesc.RenderTargetDesc[i].SrcBlend == BF_INV_DEST_COLOR ||
		   mData.BlendStateDesc.RenderTargetDesc[i].SrcBlend == BF_DEST_ALPHA ||
		   mData.BlendStateDesc.RenderTargetDesc[i].SrcBlend == BF_INV_DEST_ALPHA)
		{
			transparent = true;
		}
	}

	return transparent;
}

template <bool Core>
const GpuProgramCreateInformation& TPass<Core>::GetProgramDesc(bs::GpuProgramType type) const
{
	switch(type)
	{
	default:
	case GPT_VERTEX_PROGRAM:
		return mData.VertexProgramDesc;
	case GPT_FRAGMENT_PROGRAM:
		return mData.FragmentProgramDesc;
	case GPT_GEOMETRY_PROGRAM:
		return mData.GeometryProgramDesc;
	case GPT_HULL_PROGRAM:
		return mData.HullProgramDesc;
	case GPT_DOMAIN_PROGRAM:
		return mData.DomainProgramDesc;
	case GPT_COMPUTE_PROGRAM:
		return mData.ComputeProgramDesc;
	}
}

template <bool Core>
void TPass<Core>::CreatePipelineState()
{
	if(IsCompute())
	{
		SPtr<GpuProgramType> program = GpuProgramType::Create(mData.ComputeProgramDesc);
		mComputePipelineState = ComputePipelineStateType::Create(program);
	}
	else
	{
		PipelineStateDescType desc;

		if(!mData.VertexProgramDesc.Source.empty())
			desc.VertexProgram = GpuProgramType::Create(mData.VertexProgramDesc);

		if(!mData.FragmentProgramDesc.Source.empty())
			desc.FragmentProgram = GpuProgramType::Create(mData.FragmentProgramDesc);

		if(!mData.GeometryProgramDesc.Source.empty())
			desc.GeometryProgram = GpuProgramType::Create(mData.GeometryProgramDesc);

		if(!mData.HullProgramDesc.Source.empty())
			desc.HullProgram = GpuProgramType::Create(mData.HullProgramDesc);

		if(!mData.DomainProgramDesc.Source.empty())
			desc.DomainProgram = GpuProgramType::Create(mData.DomainProgramDesc);

		desc.BlendState = BlendStateType::Create(mData.BlendStateDesc);
		desc.RasterizerState = RasterizerStateType::Create(mData.RasterizerStateDesc);
		desc.DepthStencilState = DepthStencilStateType::Create(mData.DepthStencilStateDesc);

		mGraphicsPipelineState = GraphicsPipelineStateType::Create(desc);
	}
}

template <bool Core>
template <class P>
void TPass<Core>::RttiEnumFields(P p)
{
	p(mGraphicsPipelineState);
	p(mComputePipelineState);
}

template class TPass<false>;
template class TPass<true>;

Pass::Pass(const PASS_DESC& desc)
	: TPass(desc)
{}

SPtr<ct::Pass> Pass::GetCore() const
{
	return std::static_pointer_cast<ct::Pass>(mCoreSpecific);
}

SPtr<ct::CoreObject> Pass::CreateCore() const
{
	ct::Pass* pass = new(B3DAllocate<ct::Pass>()) ct::Pass(mData);

	SPtr<ct::Pass> passPtr = B3DMakeSharedFromExisting(pass);
	passPtr->SetThisPtrInternal(passPtr);

	return passPtr;
}

void Pass::Compile()
{
	if(mComputePipelineState || mGraphicsPipelineState)
		return; // Already compiled

	// Note: It's possible (and quite likely) the pass has already been compiled on the core thread, so this will
	// unnecessarily recompile it. However syncing them in a clean way is not trivial hard and this method is currently
	// not being used much (at all) to warrant a complex solution. Something to keep in mind for later though.
	CreatePipelineState();

	MarkCoreDirty();
	CoreObject::SyncToCore();
}

CoreSyncData Pass::SyncToCore(FrameAlloc* allocator)
{
	u32 size = CoreSyncGetSize(*this);
	u8* data = allocator->Alloc(size);

	Bitstream stream(data, size);
	B3DCoreSyncWrite(*this, stream);

	return CoreSyncData(data, size);
}

SPtr<Pass> Pass::Create(const PASS_DESC& desc)
{
	Pass* newPass = new(B3DAllocate<Pass>()) Pass(desc);
	SPtr<Pass> newPassPtr = B3DMakeCoreFromExisting<Pass>(newPass);
	newPassPtr->SetThisPtrInternal(newPassPtr);
	newPassPtr->Initialize();

	return newPassPtr;
}

SPtr<Pass> Pass::CreateEmpty()
{
	Pass* newPass = new(B3DAllocate<Pass>()) Pass();
	SPtr<Pass> newPassPtr = B3DMakeCoreFromExisting<Pass>(newPass);
	newPassPtr->SetThisPtrInternal(newPassPtr);

	return newPassPtr;
}

RTTITypeBase* Pass::GetRttiStatic()
{
	return PassRTTI::Instance();
}

RTTITypeBase* Pass::GetRtti() const
{
	return Pass::GetRttiStatic();
}

namespace bs { namespace ct
{
Pass::Pass(const PASS_DESC& desc)
	: TPass(desc)
{}

void Pass::Compile()
{
	if(mComputePipelineState || mGraphicsPipelineState)
		return; // Already compiled

	CreatePipelineState();
}

void Pass::SyncToCore(const CoreSyncData& data)
{
	Bitstream stream(data.GetBuffer(), data.GetBufferSize());
	B3DCoreSyncRead(*this, stream);
}

SPtr<Pass> Pass::Create(const PASS_DESC& desc)
{
	Pass* newPass = new(B3DAllocate<Pass>()) Pass(desc);
	SPtr<Pass> newPassPtr = B3DMakeSharedFromExisting<Pass>(newPass);
	newPassPtr->SetThisPtrInternal(newPassPtr);
	newPassPtr->Initialize();

	return newPassPtr;
}
}}
