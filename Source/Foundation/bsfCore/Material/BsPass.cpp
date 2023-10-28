//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/BsPass.h"

#include "BsCoreApplication.h"
#include "Private/RTTI/BsPassRTTI.h"
#include "Material/BsMaterial.h"
#include "RenderAPI/BsGpuParameters.h"
#include "RenderAPI/BsGpuProgram.h"
#include "RenderAPI/BsGpuPipelineState.h"
#include "CoreThread/BsCoreObjectSync.h"
#include "RenderAPI/BsGpuDevice.h"

using namespace bs;

template <bool Core>
TPass<Core>::TPass()
{
	mData.StencilRefValue = 0;
}

template <bool Core>
TPass<Core>::TPass(const PassCreateInformation& data)
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
		if(mData.BlendStateDesc.RenderTargets[i].ColorDestinationFactor != BF_ZERO ||
		   mData.BlendStateDesc.RenderTargets[i].ColorSourceFactor == BF_DEST_COLOR ||
		   mData.BlendStateDesc.RenderTargets[i].ColorSourceFactor == BF_INV_DEST_COLOR ||
		   mData.BlendStateDesc.RenderTargets[i].ColorSourceFactor == BF_DEST_ALPHA ||
		   mData.BlendStateDesc.RenderTargets[i].ColorSourceFactor == BF_INV_DEST_ALPHA)
		{
			transparent = true;
		}
	}

	return transparent;
}

template <bool Core>
const GpuProgramCreateInformation& TPass<Core>::GetGpuProgramCreateInformation(bs::GpuProgramType type) const
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
	const SPtr<GpuDevice>& device = GetCoreApplication().GetPrimaryGpuDevice();

	if(IsCompute())
	{
		GpuComputePipelineStateCreateInformation createInformation;
		createInformation.Program = device->CreateGpuProgram(mData.ComputeProgramDesc);

		mComputePipelineState = device->CreateGpuComputePipelineState(createInformation);
	}
	else
	{
		GpuGraphicsPipelineStateCreateInformation createInformation;

		if(!mData.VertexProgramDesc.Source.empty())
			createInformation.VertexProgram = device->CreateGpuProgram(mData.VertexProgramDesc);

		if(!mData.FragmentProgramDesc.Source.empty())
			createInformation.FragmentProgram = device->CreateGpuProgram(mData.FragmentProgramDesc);

		if(!mData.GeometryProgramDesc.Source.empty())
			createInformation.GeometryProgram = device->CreateGpuProgram(mData.GeometryProgramDesc);

		if(!mData.HullProgramDesc.Source.empty())
			createInformation.HullProgram = device->CreateGpuProgram(mData.HullProgramDesc);

		if(!mData.DomainProgramDesc.Source.empty())
			createInformation.DomainProgram = device->CreateGpuProgram(mData.DomainProgramDesc);

		createInformation.BlendState = mData.BlendStateDesc;
		createInformation.RasterizerState = mData.RasterizerStateDesc;
		createInformation.DepthStencilState = mData.DepthStencilStateDesc;

		mGraphicsPipelineState = device->CreateGpuGraphicsPipelineState(createInformation);
	}
}

template class TPass<false>;
template class TPass<true>;

namespace bs
{
	B3D_SYNC_BLOCK_BEGIN(Pass, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(mGraphicsPipelineState)
		B3D_SYNC_BLOCK_ENTRY(mComputePipelineState)
	B3D_SYNC_BLOCK_END
}

Pass::Pass(const PassCreateInformation& createInformation)
	: TPass(createInformation)
{}

SPtr<ct::Pass> Pass::GetCore() const
{
	return std::static_pointer_cast<ct::Pass>(mCoreSpecific);
}

SPtr<ct::CoreObject> Pass::CreateCore() const
{
	ct::Pass* pass = new(B3DAllocate<ct::Pass>()) ct::Pass(mData);

	SPtr<ct::Pass> passPtr = B3DMakeSharedFromExisting(pass);
	passPtr->SetShared(passPtr);

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

	// TODO - Non-core Pass possibly shouldn't even hold onto the pipeline states. The sync can just include a request to compile.

	MarkCoreDirty();
	CoreObject::SyncToCore();
}

CoreSyncPacket* Pass::CreateSyncPacket(FrameAllocator& allocator, u32 flags)
{
	return allocator.Construct<SyncPacket>(*this, allocator, flags);
}

SPtr<Pass> Pass::Create(const PassCreateInformation& desc)
{
	Pass* newPass = new(B3DAllocate<Pass>()) Pass(desc);
	SPtr<Pass> newPassPtr = B3DMakeSharedFromExisting<Pass>(newPass);
	newPassPtr->SetShared(newPassPtr);
	newPassPtr->Initialize();

	return newPassPtr;
}

SPtr<Pass> Pass::CreateEmpty()
{
	Pass* newPass = new(B3DAllocate<Pass>()) Pass();
	SPtr<Pass> newPassPtr = B3DMakeSharedFromExisting<Pass>(newPass);
	newPassPtr->SetShared(newPassPtr);

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
Pass::Pass(const PassCreateInformation& desc)
	: TPass(desc)
{}

void Pass::Compile()
{
	if(mComputePipelineState || mGraphicsPipelineState)
		return; // Already compiled

	CreatePipelineState();
}

void Pass::SyncToCore(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<CoreSyncPacket>();
	if(!syncPacket)
		return;

	syncPacket->ApplySyncData(this);
}

SPtr<Pass> Pass::Create(const PassCreateInformation& createInformation)
{
	Pass* newPass = new(B3DAllocate<Pass>()) Pass(createInformation);
	SPtr<Pass> newPassPtr = B3DMakeSharedFromExisting<Pass>(newPass);
	newPassPtr->SetShared(newPassPtr);
	newPassPtr->Initialize();

	return newPassPtr;
}

SPtr<Pass> Pass::CreateEmpty()
{
	Pass* const pass = new(B3DAllocate<Pass>()) Pass();
	SPtr<Pass> passShared = B3DMakeSharedFromExisting(pass);
	passShared->SetShared(passShared);
	passShared->Initialize();

	return passShared;
}

RTTITypeBase* Pass::GetRttiStatic()
{
	return CorePassRTTI::Instance();
}

RTTITypeBase* Pass::GetRtti() const
{
	return Pass::GetRttiStatic();
}
}}
