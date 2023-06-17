//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsSkybox.h"

#include "BsCoreApplication.h"
#include "Private/RTTI/BsSkyboxRTTI.h"
#include "Scene/BsSceneObject.h"
#include "Image/BsTexture.h"
#include "Renderer/BsRenderer.h"
#include "Utility/BsUUID.h"
#include "Renderer/BsIBLUtility.h"
#include "CoreThread/BsCoreObjectSync.h"
#include "Profiling/BsProfilerGPU.h"
#include "RenderAPI/BsGpuDevice.h"

using namespace bs;

template <bool Core>
template <class P>
void TSkybox<Core>::RttiEnumFields(P p)
{
	p(mBrightness);
	p(mTexture);
}

Skybox::Skybox()
{
	// This shouldn't normally happen, as filtered textures are generated when a radiance texture is assigned, but
	// we check for it anyway (something could have gone wrong).
	if(mTexture.IsLoaded())
	{
		if(mFilteredRadiance == nullptr || mIrradiance == nullptr)
			FilterTexture();
	}
}

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
		cubemapDesc.Width = ct::IBLUtility::kReflectionCubemapSize;
		cubemapDesc.Height = ct::IBLUtility::kReflectionCubemapSize;
		cubemapDesc.MipMapCount = PixelUtil::GetMaxMipmaps(cubemapDesc.Width, cubemapDesc.Height, 1, cubemapDesc.Format);
		cubemapDesc.Usage = TU_STATIC | TU_RENDERTARGET;

		mFilteredRadiance = Texture::CreateShared(cubemapDesc);
	}

	{
		TextureCreateInformation irradianceCubemapDesc;
		irradianceCubemapDesc.Name = "Skybox irradiance cubemap";
		irradianceCubemapDesc.Type = TEX_TYPE_CUBE_MAP;
		irradianceCubemapDesc.Format = PF_RG11B10F;
		irradianceCubemapDesc.Width = ct::IBLUtility::kIrradianceCubemapSize;
		irradianceCubemapDesc.Height = ct::IBLUtility::kIrradianceCubemapSize;
		irradianceCubemapDesc.MipMapCount = 0;
		irradianceCubemapDesc.Usage = TU_STATIC | TU_RENDERTARGET;

		mIrradiance = Texture::CreateShared(irradianceCubemapDesc);
	}

	auto renderComplete = [this]()
	{
		mRendererTask = nullptr;
	};

	SPtr<ct::Skybox> coreSkybox = GetCore();
	SPtr<ct::Texture> coreFilteredRadiance = mFilteredRadiance->GetCore();
	SPtr<ct::Texture> coreIrradiance = mIrradiance->GetCore();

	auto filterSkybox = [coreFilteredRadiance, coreIrradiance, coreSkybox](ct::GpuCommandBufferPool& commandBufferPool)
	{
		const SPtr<ct::GpuCommandBuffer> commandBuffer = commandBufferPool.Create(ct::GpuCommandBufferCreateInformation::Create("FilterSkybox"));

		GetProfilerGPU().BeginSample(*commandBuffer, "FilterSkybox");

		// Filter radiance
		ct::GetIBLUtility().ScaleCubemap(*commandBuffer, coreSkybox->GetTexture(), 0, coreFilteredRadiance, 0);
		ct::GetIBLUtility().FilterCubemapForSpecular(*commandBuffer, coreFilteredRadiance, nullptr);

		coreSkybox->mFilteredRadiance = coreFilteredRadiance;

		// Generate irradiance
		ct::GetIBLUtility().FilterCubemapForIrradiance(*commandBuffer, coreSkybox->GetTexture(), coreIrradiance);
		coreSkybox->mIrradiance = coreIrradiance;

		GetProfilerGPU().EndSample(*commandBuffer, "FilterSkybox");
		const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
		gpuDevice->SubmitCommandBuffer(commandBuffer);

		return true;
	};

	mRendererTask = ct::RendererTask::Create("SkyboxFilter", filterSkybox);

	mRendererTask->OnComplete.Connect(renderComplete);
	ct::GetRenderer()->AddTask(mRendererTask);
}

void Skybox::SetTexture(const HTexture& texture)
{
	mTexture = texture;

	mFilteredRadiance = nullptr;
	mIrradiance = nullptr;

	if(mTexture.IsLoaded())
		FilterTexture();

	MarkCoreDirtyInternal((ActorDirtyFlag)SkyboxDirtyFlag::Texture);
}

SPtr<ct::Skybox> Skybox::GetCore() const
{
	return std::static_pointer_cast<ct::Skybox>(mCoreSpecific);
}

SPtr<Skybox> Skybox::CreateEmpty()
{
	Skybox* skybox = new(B3DAllocate<Skybox>()) Skybox();
	SPtr<Skybox> skyboxPtr = B3DMakeCoreFromExisting<Skybox>(skybox);
	skyboxPtr->SetShared(skyboxPtr);

	return skyboxPtr;
}

SPtr<Skybox> Skybox::Create()
{
	SPtr<Skybox> skyboxPtr = CreateEmpty();
	skyboxPtr->Initialize();

	return skyboxPtr;
}

SPtr<ct::CoreObject> Skybox::CreateCore() const
{
	SPtr<ct::Texture> radiance;
	if(mTexture.IsLoaded(false))
		radiance = mTexture->GetCore();

	SPtr<ct::Texture> filteredRadiance;
	if(mFilteredRadiance)
		filteredRadiance = mFilteredRadiance->GetCore();

	SPtr<ct::Texture> irradiance;
	if(mIrradiance)
		irradiance = mIrradiance->GetCore();

	ct::Skybox* skybox = new(B3DAllocate<ct::Skybox>()) ct::Skybox(radiance, filteredRadiance, irradiance);
	SPtr<ct::Skybox> skyboxPtr = B3DMakeSharedFromExisting<ct::Skybox>(skybox);
	skyboxPtr->SetShared(skyboxPtr);

	return skyboxPtr;
}

CoreSyncData Skybox::SyncToCore(FrameAlloc* allocator)
{
	u32 size = 0;
	size += B3DRTTISize(GetCoreDirtyFlags()).Bytes;
	size += CoreSyncGetSize((SceneActor&)*this);
	size += CoreSyncGetSize(*this);

	u8* buffer = allocator->Alloc(size);

	Bitstream stream(buffer, size);
	B3DRTTIWrite(GetCoreDirtyFlags(), stream);
	B3DCoreSyncWrite((SceneActor&)*this, stream);
	B3DCoreSyncWrite(*this, stream);

	return CoreSyncData(buffer, size);
}

void Skybox::MarkCoreDirtyInternal(ActorDirtyFlag flags)
{
	MarkCoreDirty((u32)flags);
}

RTTITypeBase* Skybox::GetRttiStatic()
{
	return SkyboxRTTI::Instance();
}

RTTITypeBase* Skybox::GetRtti() const
{
	return Skybox::GetRttiStatic();
}

namespace bs { namespace ct
{
Skybox::Skybox(const SPtr<Texture>& radiance, const SPtr<Texture>& filteredRadiance, const SPtr<Texture>& irradiance)
	: mFilteredRadiance(filteredRadiance), mIrradiance(irradiance)
{
	mTexture = radiance;
}

Skybox::~Skybox()
{
	GetRenderer()->NotifySkyboxRemoved(this);
}

void Skybox::Initialize()
{
	GetRenderer()->NotifySkyboxAdded(this);

	CoreObject::Initialize();
}

void Skybox::SyncToCore(const CoreSyncData& data, FrameAlloc& allocator)
{
	Bitstream stream(data.GetBuffer(), data.GetBufferSize());

	SkyboxDirtyFlag dirtyFlags;
	bool oldIsActive = mActive;

	B3DRTTIRead(dirtyFlags, stream);
	B3DCoreSyncRead((SceneActor&)*this, stream);
	B3DCoreSyncRead(*this, stream);

	if(oldIsActive != mActive)
	{
		if(mActive)
			GetRenderer()->NotifySkyboxAdded(this);
		else
			GetRenderer()->NotifySkyboxRemoved(this);
	}
	else
	{
		if(dirtyFlags != SkyboxDirtyFlag::Texture)
		{
			GetRenderer()->NotifySkyboxRemoved(this);
			GetRenderer()->NotifySkyboxAdded(this);
		}
	}
}
}}
