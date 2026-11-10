//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/B3DPass.h"

#include "B3DApplication.h"
#include "RTTI/B3DPassRTTI.h"
#include "Material/B3DMaterial.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "GpuBackend/B3DGpuProgram.h"
#include "GpuBackend/B3DGpuPipelineState.h"
#include "CoreObject/B3DCoreObjectSync.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"

using namespace b3d;

template <bool IsRenderProxy>
TPass<IsRenderProxy>::TPass()
{
	mData.StencilRefValue = 0;
}

template <bool IsRenderProxy>
TPass<IsRenderProxy>::TPass(const PassCreateInformation& createInformation)
	: mData((PassInformation)createInformation)
{
}

template <bool IsRenderProxy>
bool TPass<IsRenderProxy>::HasBlending() const
{
	bool transparent = false;

	for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
	{
		// Transparent if destination color is taken into account
		if(mData.BlendStateInformation.RenderTargets[i].ColorDestinationFactor != BF_ZERO ||
		   mData.BlendStateInformation.RenderTargets[i].ColorSourceFactor == BF_DEST_COLOR ||
		   mData.BlendStateInformation.RenderTargets[i].ColorSourceFactor == BF_INV_DEST_COLOR ||
		   mData.BlendStateInformation.RenderTargets[i].ColorSourceFactor == BF_DEST_ALPHA ||
		   mData.BlendStateInformation.RenderTargets[i].ColorSourceFactor == BF_INV_DEST_ALPHA)
		{
			transparent = true;
		}
	}

	return transparent;
}

template <bool IsRenderProxy>
bool TPass<IsRenderProxy>::IsCompute() const
{
	for(const GpuProgramCreateInformation& program : mData.Programs)
	{
		if(program.Type == GPT_COMPUTE_PROGRAM)
			return true;
	}

	return false;
}

template <bool IsRenderProxy>
bool TPass<IsRenderProxy>::IsRayTracing() const
{
	for(const GpuProgramCreateInformation& program : mData.Programs)
	{
		if(IsRayTracingProgramType(program.Type))
			return true;
	}

	return false;
}

template <bool IsRenderProxy>
const GpuProgramCreateInformation& TPass<IsRenderProxy>::GetGpuProgramCreateInformation(b3d::GpuProgramType type) const
{
	for(const GpuProgramCreateInformation& program : mData.Programs)
	{
		if(program.Type == type)
			return program;
	}

	static const GpuProgramCreateInformation kNoProgram;
	return kNoProgram;
}

template <bool IsRenderProxy>
TShared<GpuProgram> TPass<IsRenderProxy>::GetProgram(const StringView& entryPoint) const
{
	for(u32 programIndex = 0; programIndex < (u32)mPrograms.size(); programIndex++)
	{
		if(mData.Programs[programIndex].EntryPoint == entryPoint)
			return mPrograms[programIndex];
	}

	return nullptr;
}

template <bool IsRenderProxy>
void TPass<IsRenderProxy>::CreatePrograms()
{
	if(mAreProgramsCreated)
		return;

	mAreProgramsCreated = true;
	mPrograms.resize(mData.Programs.size());

	const TShared<GpuDevice>& device = GetApplication().GetPrimaryGpuDevice();
	if(IsRayTracing())
	{
		if(device->GetCapabilities().HasCapability(RSC_RAY_TRACING_PIPELINE))
		{
			for(u32 programIndex = 0; programIndex < (u32)mData.Programs.size(); programIndex++)
			{
				// Keep only programs the device can run: callable programs need SupportsCallablePrograms, for example
				TShared<GpuProgram> program = device->CreateGpuProgram(mData.Programs[programIndex]);
				if(program != nullptr && program->IsSupported())
					mPrograms[programIndex] = std::move(program);
			}
		}
	}
	else
	{
		GpuComputePipelineStateCreateInformation computeInformation;
		GpuGraphicsPipelineStateCreateInformation graphicsInformation;
		for(u32 programIndex = 0; programIndex < (u32)mData.Programs.size(); programIndex++)
		{
			mPrograms[programIndex] = device->CreateGpuProgram(mData.Programs[programIndex]);

			const TShared<GpuProgram>& program = mPrograms[programIndex];
			switch(mData.Programs[programIndex].Type)
			{
			case GPT_VERTEX_PROGRAM: graphicsInformation.VertexProgram = program; break;
			case GPT_FRAGMENT_PROGRAM: graphicsInformation.FragmentProgram = program; break;
			case GPT_GEOMETRY_PROGRAM: graphicsInformation.GeometryProgram = program; break;
			case GPT_HULL_PROGRAM: graphicsInformation.HullProgram = program; break;
			case GPT_DOMAIN_PROGRAM: graphicsInformation.DomainProgram = program; break;
			case GPT_COMPUTE_PROGRAM: computeInformation.Program = program; break;
			default: break;
			}
		}

		if(IsCompute())
			mComputePipelineState = device->CreateGpuComputePipelineState(computeInformation);
		else
		{
			graphicsInformation.BlendState = mData.BlendStateInformation;
			graphicsInformation.RasterizerState = mData.RasterizerStateInformation;
			graphicsInformation.DepthStencilState = mData.DepthStencilStateInformation;

			mGraphicsPipelineState = device->CreateGpuGraphicsPipelineState(graphicsInformation);
		}
	}
}

namespace b3d
{
	template class TPass<false>;
	template class TPass<true>;
} // namespace b3d

RTTIType* PassInformation::GetRttiStatic()
{
	return PassInformationRTTI::Instance();
}

RTTIType* PassInformation::GetRtti() const
{
	return PassInformation::GetRttiStatic();
}

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(Pass, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(mGraphicsPipelineState)
		B3D_SYNC_BLOCK_ENTRY(mComputePipelineState)
		B3D_SYNC_BLOCK_ENTRY(mPrograms)
	B3D_SYNC_BLOCK_END
}

Pass::Pass(const PassCreateInformation& createInformation)
	: TPass((PassInformation)createInformation)
{}

TShared<render::RenderProxy> Pass::CreateRenderProxy() const
{
	render::Pass* renderProxy = new(B3DAllocate<render::Pass>()) render::Pass(mData);

	TShared<render::Pass> renderProxyShared = B3DMakeSharedFromExisting(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	return renderProxyShared;
}

void Pass::Compile()
{
	if(mAreProgramsCreated)
		return; // Already compiled

	// Ray tracing passes have no pipeline state to create or sync
	if(IsRayTracing())
		return;

	// Note: It's possible (and quite likely) the pass has already been compiled on the render thread, so this will
	// unnecessarily recompile it. However syncing them in a clean way is not trivial hard and this method is currently
	// not being used much (at all) to warrant a complex solution. Something to keep in mind for later though.
	CreatePrograms();

	// TODO - Non-render proxy Pass possibly shouldn't even hold onto the pipeline states. The sync can just include a request to compile.

	MarkRenderProxyDataDirty();
	CoreObject::SyncToRenderProxy();
}

RenderProxySyncPacket* Pass::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	return allocator.Construct<SyncPacket>(*this, allocator, flags);
}

TShared<Pass> Pass::Create(const PassCreateInformation& createInformation)
{
	Pass* newPass = new(B3DAllocate<Pass>()) Pass(createInformation);
	TShared<Pass> newPassPtr = B3DMakeSharedFromExisting<Pass>(newPass);
	newPassPtr->SetShared(newPassPtr);
	newPassPtr->Initialize();

	return newPassPtr;
}

TShared<Pass> Pass::CreateEmpty()
{
	Pass* newPass = new(B3DAllocate<Pass>()) Pass();
	TShared<Pass> newPassPtr = B3DMakeSharedFromExisting<Pass>(newPass);
	newPassPtr->SetShared(newPassPtr);

	return newPassPtr;
}

RTTIType* Pass::GetRttiStatic()
{
	return PassRTTI::Instance();
}

RTTIType* Pass::GetRtti() const
{
	return Pass::GetRttiStatic();
}

namespace b3d { namespace render
{
Pass::Pass(const PassCreateInformation& createInformation)
	: TPass(createInformation)
{}

void Pass::Compile()
{
	// The main thread pass may have compiled already and synced its programs and pipeline state
	if(mComputePipelineState || mGraphicsPipelineState)
		return;

	CreatePrograms();
}

void Pass::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<RenderProxySyncPacket>();
	if(!syncPacket)
		return;

	syncPacket->ApplySyncData(this);
}

TShared<Pass> Pass::Create(const PassCreateInformation& createInformation)
{
	Pass* newPass = new(B3DAllocate<Pass>()) Pass(createInformation);
	TShared<Pass> newPassPtr = B3DMakeSharedFromExisting<Pass>(newPass);
	newPassPtr->SetShared(newPassPtr);
	newPassPtr->Initialize();

	return newPassPtr;
}

TShared<Pass> Pass::CreateEmpty()
{
	Pass* const pass = new(B3DAllocate<Pass>()) Pass();
	TShared<Pass> passShared = B3DMakeSharedFromExisting(pass);
	passShared->SetShared(passShared);
	passShared->Initialize();

	return passShared;
}

RTTIType* Pass::GetRttiStatic()
{
	return PassRenderProxyRTTI::Instance();
}

RTTIType* Pass::GetRtti() const
{
	return Pass::GetRttiStatic();
}
}}
