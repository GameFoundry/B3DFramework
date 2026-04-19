//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/B3DSkybox.h"
#include "B3DApplication.h"
#include "CoreObject/B3DCoreObjectSync.h"
#include "Image/B3DTexture.h"
#include "RTTI/B3DSkyboxRTTI.h"
#include "Profiling/B3DProfilerGPU.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"
#include "RenderAPI/B3DGpuDevice.h"
#include "Renderer/B3DIBLUtility.h"
#include "Renderer/B3DRenderer.h"
#include "Renderer/B3DRendererScene.h"
#include "Scene/B3DSceneInstance.h"

using namespace b3d;

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(Skybox, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(mBrightness)
		B3D_SYNC_BLOCK_ENTRY(mTexture)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM_SETTER(bool, mActive)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM_SETTER(SPtr<SceneInstance>, mSceneInstance)
	B3D_SYNC_BLOCK_END
}

template <bool IsRenderProxy>
void TSkybox<IsRenderProxy>::MarkRenderProxyDataDirty(ComponentDirtyFlag flag)
{
	if constexpr(!IsRenderProxy)
		CoreObject::MarkRenderProxyDataDirty((u32)flag);
}

template TSkybox<true>;
template TSkybox<false>;

Skybox::Skybox(const HSceneObject& parent)
	: Component(parent)
{
	SetFlag(ComponentFlag::AlwaysRun, true);
	SetName("Skybox");
}

Skybox::Skybox()
	: Skybox(nullptr)
{ }

void Skybox::Initialize()
{
	SetShared(B3DStaticGameObjectCast<Skybox>(mThisHandle).GetShared());

	Component::Initialize();
	CoreObject::Initialize();
}

void Skybox::OnCreated()
{
	// This shouldn't normally happen, as filtered textures are generated when a radiance texture is assigned, but
	// we check for it anyway (something could have gone wrong).
	if(mTexture.IsLoaded())
	{
		if(mFilteredRadiance == nullptr || mIrradiance == nullptr)
			FilterTexture();
	}
}

void Skybox::OnEnabled()
{
	MarkRenderProxyDataDirty();
}

void Skybox::OnDisabled()
{
	MarkRenderProxyDataDirty();
}

void Skybox::OnDestroyed()
{
	if(mRendererTask)
		mRendererTask->Cancel();

	CoreObject::Destroy();
}

void Skybox::SetTexture(const HTexture& texture)
{
	mTexture = texture;

	mFilteredRadiance = nullptr;
	mIrradiance = nullptr;

	if(mTexture.IsLoaded())
		FilterTexture();

	MarkRenderProxyDataDirty();
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
		cubemapDesc.Usage = TextureUsageFlag::StoreOnGPU | TextureUsageFlag::RenderTarget;

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
		irradianceCubemapDesc.Usage = TextureUsageFlag::StoreOnGPU | TextureUsageFlag::RenderTarget;

		mIrradiance = Texture::CreateShared(irradianceCubemapDesc);
	}

	auto fnRenderComplete = [this]()
	{
		mRendererTask = nullptr;
	};

	SPtr<render::Skybox> skyboxRenderProxy = B3DGetRenderProxy(this);
	SPtr<render::Texture> filteredRadianceRenderProxy = B3DGetRenderProxy(mFilteredRadiance);
	SPtr<render::Texture> irradianceRenderProxy = B3DGetRenderProxy(mIrradiance);

	auto fnFilterSkybox = [filteredRadianceRenderProxy, irradianceRenderProxy, skyboxRenderProxy](render::GpuCommandBufferPool& commandBufferPool)
	{
		const SPtr<render::GpuCommandBuffer> commandBuffer = commandBufferPool.Create(render::GpuCommandBufferCreateInformation::Create("FilterSkybox"));
		SPtr<GpuCommandBufferProfiler> commandBufferProfiler = GetGpuProfiler().CreateCommandBufferProfiler(*commandBuffer);

		commandBufferProfiler->BeginSample(*commandBuffer, "FilterSkybox");

		// Filter radiance
		render::GetIBLUtility().ScaleCubemap(*commandBuffer, skyboxRenderProxy->GetTexture(), 0, filteredRadianceRenderProxy, 0);
		render::GetIBLUtility().FilterCubemapForSpecular(*commandBuffer, filteredRadianceRenderProxy, nullptr);

		skyboxRenderProxy->mFilteredRadiance = filteredRadianceRenderProxy;

		// Generate irradiance
		render::GetIBLUtility().FilterCubemapForIrradiance(*commandBuffer, skyboxRenderProxy->GetTexture(), irradianceRenderProxy);
		skyboxRenderProxy->mIrradiance = irradianceRenderProxy;

		commandBufferProfiler->EndSample(*commandBuffer);
		GetGpuProfiler().ResolveProfileWhenReady("FilterSkybox", commandBufferProfiler);

		const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
		gpuDevice->SubmitCommandBuffer(commandBuffer);

		return true;
	};

	mRendererTask = render::RendererTask::Create("SkyboxFilter", fnFilterSkybox);

	mRendererTask->OnComplete.Connect(fnRenderComplete);
	render::GetRenderer()->AddTask(mRendererTask);
}

SPtr<render::RenderProxy> Skybox::CreateRenderProxy() const
{
	const SPtr<SceneInstance>& scene = SceneObject()->GetScene();
	SPtr<render::Texture> radiance = B3DGetRenderProxy(mTexture);
	SPtr<render::Texture> filteredRadiance = B3DGetRenderProxy(mFilteredRadiance);
	SPtr<render::Texture> irradiance = B3DGetRenderProxy(mIrradiance);

	render::Skybox* renderProxy = new(B3DAllocate<render::Skybox>()) render::Skybox(B3DGetRenderProxy(scene), radiance, filteredRadiance, irradiance);
	SPtr<render::Skybox> renderProxyShared = B3DMakeSharedFromExisting<render::Skybox>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	return renderProxyShared;
}

RenderProxySyncPacket* Skybox::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	SyncPacket* const syncPacket = allocator.Construct<SyncPacket>(*this, allocator, flags);
	syncPacket->mActive = GetEnabled();
	syncPacket->mSceneInstance = B3DGetRenderProxy(SceneObject()->GetScene());

	return syncPacket;
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
	: mSceneInstance(scene), mFilteredRadiance(filteredRadiance), mIrradiance(irradiance)
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

	bool previousIsActive = mActive;
	syncPacket->ApplySyncData(this);

	const SPtr<RendererScene>& rendererScene = mSceneInstance->GetRendererScene();
	if(previousIsActive != mActive)
	{
		if(mActive)
			rendererScene->RegisterSkybox(this);
		else
			rendererScene->UnregisterSkybox(this);
	}
	else
	{
		rendererScene->UnregisterSkybox(this);
		rendererScene->RegisterSkybox(this);
	}
}
}}
