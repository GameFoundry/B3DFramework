//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsSkybox.h"

#include "BsCoreApplication.h"
#include "BsRendererScene.h"
#include "Private/RTTI/BsSkyboxRTTI.h"
#include "Scene/BsSceneObject.h"
#include "Image/BsTexture.h"
#include "Renderer/BsRenderer.h"
#include "Utility/BsUUID.h"
#include "Renderer/BsIBLUtility.h"
#include "CoreObject/BsCoreObjectSync.h"
#include "Profiling/BsProfilerGPU.h"
#include "RenderAPI/BsGpuDevice.h"
#include "Scene/BsSceneManager.h"

using namespace b3d;

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(Skybox, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(mBrightness)
		B3D_SYNC_BLOCK_ENTRY(mTexture)
		B3D_SYNC_BLOCK_ENTRY_PACKET_BASE(SceneActor, SceneActorPacket)
	B3D_SYNC_BLOCK_END
}

template <bool IsRenderProxy>
TSkybox<IsRenderProxy>::TSkybox(const SPtr<SceneInstanceType>& scene)
	:Super(scene)
{ }

template TSkybox<true>;
template TSkybox<false>;

Skybox::Skybox(const SPtr<SceneInstance>& scene)
	:TSkybox(scene)
{
	// This shouldn't normally happen, as filtered textures are generated when a radiance texture is assigned, but
	// we check for it anyway (something could have gone wrong).
	if(mTexture.IsLoaded())
	{
		if(mFilteredRadiance == nullptr || mIrradiance == nullptr)
			FilterTexture();
	}
}

Skybox::Skybox()
	:Skybox(nullptr)
{ }

Skybox::~Skybox()
{
	if(mRendererTask != nullptr)
		mRendererTask->Cancel();
}

void Skybox::FilterTexture()
{
	// If previous rendering task exists, cancel it
	if(mRendererTask != nullptr)
		mRendererTask->Cancel();

	{
		TextureCreateInformation cubemapDesc;
		cubemapDesc.Name = "Skybox filtered radiance cubemap";
		cubemapDesc.Type = TEX_TYPE_CUBE_MAP;
		cubemapDesc.Format = PF_RG11B10F;
		cubemapDesc.Width = render::IBLUtility::kReflectionCubemapSize;
		cubemapDesc.Height = render::IBLUtility::kReflectionCubemapSize;
		cubemapDesc.MipMapCount = PixelUtility::GetMipmapCount(cubemapDesc.Width, cubemapDesc.Height, 1, cubemapDesc.Format);
		cubemapDesc.Usage = TU_STATIC | TU_RENDERTARGET;

		mFilteredRadiance = Texture::CreateShared(cubemapDesc);
	}

	{
		TextureCreateInformation irradianceCubemapDesc;
		irradianceCubemapDesc.Name = "Skybox irradiance cubemap";
		irradianceCubemapDesc.Type = TEX_TYPE_CUBE_MAP;
		irradianceCubemapDesc.Format = PF_RG11B10F;
		irradianceCubemapDesc.Width = render::IBLUtility::kIrradianceCubemapSize;
		irradianceCubemapDesc.Height = render::IBLUtility::kIrradianceCubemapSize;
		irradianceCubemapDesc.MipMapCount = 0;
		irradianceCubemapDesc.Usage = TU_STATIC | TU_RENDERTARGET;

		mIrradiance = Texture::CreateShared(irradianceCubemapDesc);
	}

	auto renderComplete = [this]()
	{
		mRendererTask = nullptr;
	};

	SPtr<render::Skybox> skyboxRenderProxy = B3DGetRenderProxy(this);
	SPtr<render::Texture> filteredRadianceRenderProxy = B3DGetRenderProxy(mFilteredRadiance);
	SPtr<render::Texture> irradianceRenderProxy = B3DGetRenderProxy(mIrradiance);

	auto filterSkybox = [filteredRadianceRenderProxy, irradianceRenderProxy, skyboxRenderProxy](render::GpuCommandBufferPool& commandBufferPool)
	{
		const SPtr<render::GpuCommandBuffer> commandBuffer = commandBufferPool.Create(render::GpuCommandBufferCreateInformation::Create("FilterSkybox"));

		GetProfilerGPU().BeginSample(*commandBuffer, "FilterSkybox");

		// Filter radiance
		render::GetIBLUtility().ScaleCubemap(*commandBuffer, skyboxRenderProxy->GetTexture(), 0, filteredRadianceRenderProxy, 0);
		render::GetIBLUtility().FilterCubemapForSpecular(*commandBuffer, filteredRadianceRenderProxy, nullptr);

		skyboxRenderProxy->mFilteredRadiance = filteredRadianceRenderProxy;

		// Generate irradiance
		render::GetIBLUtility().FilterCubemapForIrradiance(*commandBuffer, skyboxRenderProxy->GetTexture(), irradianceRenderProxy);
		skyboxRenderProxy->mIrradiance = irradianceRenderProxy;

		GetProfilerGPU().EndSample(*commandBuffer, "FilterSkybox");
		const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
		gpuDevice->SubmitCommandBuffer(commandBuffer);

		return true;
	};

	mRendererTask = render::RendererTask::Create("SkyboxFilter", filterSkybox);

	mRendererTask->OnComplete.Connect(renderComplete);
	render::GetRenderer()->AddTask(mRendererTask);
}

void Skybox::SetTexture(const HTexture& texture)
{
	mTexture = texture;

	mFilteredRadiance = nullptr;
	mIrradiance = nullptr;

	if(mTexture.IsLoaded())
		FilterTexture();

	MarkSceneActorRenderProxyDataDirty((ActorDirtyFlag)SkyboxDirtyFlag::Texture);
}

SPtr<Skybox> Skybox::CreateEmpty()
{
	Skybox* skybox = new(B3DAllocate<Skybox>()) Skybox();
	SPtr<Skybox> skyboxShared = B3DMakeSharedFromExisting<Skybox>(skybox);
	skyboxShared->SetShared(skyboxShared);

	return skyboxShared;
}

SPtr<Skybox> Skybox::Create(const SPtr<SceneInstance>& scene)
{
	Skybox* skybox = new(B3DAllocate<Skybox>()) Skybox(scene);
	SPtr<Skybox> skyboxShared = B3DMakeSharedFromExisting<Skybox>(skybox);
	skyboxShared->SetShared(skyboxShared);
	skyboxShared->Initialize();

	return skyboxShared;
}

SPtr<render::RenderProxy> Skybox::CreateRenderProxy() const
{
	SPtr<render::Texture> radiance = B3DGetRenderProxy(mTexture);
	SPtr<render::Texture> filteredRadiance = B3DGetRenderProxy(mFilteredRadiance);
	SPtr<render::Texture> irradiance = B3DGetRenderProxy(mIrradiance);

	render::Skybox* renderProxy = new(B3DAllocate<render::Skybox>()) render::Skybox(B3DGetRenderProxy(mSceneInstance), radiance, filteredRadiance, irradiance);
	SPtr<render::Skybox> renderProxyShared = B3DMakeSharedFromExisting<render::Skybox>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	return renderProxyShared;
}

RenderProxySyncPacket* Skybox::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	SyncPacket* const syncPacket = allocator.Construct<SyncPacket>(*this, allocator, flags);
	if(B3D_ENSURE(syncPacket))
		syncPacket->SceneActorPacket = CreateSceneActorRenderProxySyncPacket(allocator, flags);

	return syncPacket;
}

void Skybox::MarkSceneActorRenderProxyDataDirty(ActorDirtyFlag flags)
{
	MarkRenderProxyDataDirty((u32)flags);
}

RTTIType* Skybox::GetRttiStatic()
{
	return SkyboxRTTI::Instance();
}

RTTIType* Skybox::GetRtti() const
{
	return Skybox::GetRttiStatic();
}

namespace b3d { namespace render
{
Skybox::Skybox(const SPtr<SceneInstance>& scene, const SPtr<Texture>& radiance, const SPtr<Texture>& filteredRadiance, const SPtr<Texture>& irradiance)
	: TSkybox(scene), mFilteredRadiance(filteredRadiance), mIrradiance(irradiance)
{
	mTexture = radiance;
}

Skybox::~Skybox()
{
	const SPtr<RendererScene>& rendererScene = mSceneInstance->GetRendererScene();
	rendererScene->UnregisterSkybox(this);
}

void Skybox::Initialize()
{
	const SPtr<RendererScene>& rendererScene = mSceneInstance->GetRendererScene();
	rendererScene->RegisterSkybox(this);

	RenderProxy::Initialize();
}

void Skybox::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<b3d::Skybox::SyncPacket>();
	if(!syncPacket)
		return;

	bool oldIsActive = mActive;
	syncPacket->ApplySyncData(this);

	const SkyboxDirtyFlag dirtyFlags = (SkyboxDirtyFlag)syncPacket->Flags;

	const SPtr<RendererScene>& rendererScene = mSceneInstance->GetRendererScene();
	if(oldIsActive != mActive)
	{
		if(mActive)
			rendererScene->RegisterSkybox(this);
		else
			rendererScene->UnregisterSkybox(this);
	}
	else
	{
		if(dirtyFlags != SkyboxDirtyFlag::Texture)
		{
			rendererScene->UnregisterSkybox(this);
			rendererScene->RegisterSkybox(this);
		}
	}
}
}}
