//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Renderer/BsSkybox.h"
#include "RTTI/BsSkyboxRTTI.h"
#include "Scene/BsSceneObject.h"
#include "Allocators/BsFrameAlloc.h"
#include "Image/BsTexture.h"
#include "Renderer/BsRenderer.h"
#include "Utility/BsUUID.h"
#include "Renderer/BsIBLUtility.h"

namespace bs
{
	SkyboxBase::SkyboxBase()
		: mIsActive(true), mBrightness(1.0f)
	{ }

	template <bool Core>
	TSkybox<Core>::TSkybox()
		: SkyboxBase()
	{ }

	template class TSkybox<true>;
	template class TSkybox<false>;

	Skybox::Skybox()
	{
		// This shouldn't normally happen, as filtered textures are generated when a radiance texture is assigned, but
		// we check for it anyway (something could have gone wrong).
		if(mTexture.isLoaded())
		{
			if (mFilteredRadiance == nullptr || mIrradiance == nullptr)
				filterTexture();
		}
	}

	Skybox::~Skybox()
	{
		if (mRendererTask != nullptr)
			mRendererTask->cancel();
	}

	void Skybox::filterTexture()
	{
		// If previous rendering task exists, cancel it
		if (mRendererTask != nullptr)
			mRendererTask->cancel();

		{
			TEXTURE_DESC cubemapDesc;
			cubemapDesc.type = TEX_TYPE_CUBE_MAP;
			cubemapDesc.format = PF_RG11B10F;
			cubemapDesc.width = ct::IBLUtility::REFLECTION_CUBEMAP_SIZE;
			cubemapDesc.height = ct::IBLUtility::REFLECTION_CUBEMAP_SIZE;
			cubemapDesc.numMips = PixelUtil::getMaxMipmaps(cubemapDesc.width, cubemapDesc.height, 1, cubemapDesc.format);
			cubemapDesc.usage = TU_STATIC | TU_RENDERTARGET;

			mFilteredRadiance = Texture::_createPtr(cubemapDesc);
		}

		{
			TEXTURE_DESC irradianceCubemapDesc;
			irradianceCubemapDesc.type = TEX_TYPE_CUBE_MAP;
			irradianceCubemapDesc.format = PF_RG11B10F;
			irradianceCubemapDesc.width = ct::IBLUtility::IRRADIANCE_CUBEMAP_SIZE;
			irradianceCubemapDesc.height = ct::IBLUtility::IRRADIANCE_CUBEMAP_SIZE;
			irradianceCubemapDesc.numMips = 0;
			irradianceCubemapDesc.usage = TU_STATIC | TU_RENDERTARGET;

			mIrradiance = Texture::_createPtr(irradianceCubemapDesc);
		}

		auto renderComplete = [this]()
		{
			mRendererTask = nullptr;
		};

		SPtr<ct::Skybox> coreSkybox = getCore();
		SPtr<ct::Texture> coreFilteredRadiance = mFilteredRadiance->getCore();
		SPtr<ct::Texture> coreIrradiance = mIrradiance->getCore();

		auto filterSkybox = [coreFilteredRadiance, coreIrradiance, coreSkybox]()
		{
			// Filter radiance
			ct::gIBLUtility().scaleCubemap(coreSkybox->getTexture(), 0, coreFilteredRadiance, 0);
			ct::gIBLUtility().filterCubemapForSpecular(coreFilteredRadiance, nullptr);

			coreSkybox->mFilteredRadiance = coreFilteredRadiance;

			// Generate irradiance
			ct::gIBLUtility().filterCubemapForIrradiance(coreFilteredRadiance, coreIrradiance);
			coreSkybox->mIrradiance = coreIrradiance;

			return true;
		};

		mRendererTask = ct::RendererTask::create("SkyboxFilter", filterSkybox);

		mRendererTask->onComplete.connect(renderComplete);
		ct::gRenderer()->addTask(mRendererTask);
	}

	SPtr<ct::Skybox> Skybox::getCore() const
	{
		return std::static_pointer_cast<ct::Skybox>(mCoreSpecific);
	}

	SPtr<Skybox> Skybox::create()
	{
		Skybox* skybox = new (bs_alloc<Skybox>()) Skybox();
		SPtr<Skybox> skyboxPtr = bs_core_ptr<Skybox>(skybox);
		skyboxPtr->_setThisPtr(skyboxPtr);
		skyboxPtr->initialize();

		return skyboxPtr;
	}

	SPtr<ct::CoreObject> Skybox::createCore() const
	{
		SPtr<ct::Texture> filteredRadiance;
		if (mFilteredRadiance)
			filteredRadiance = mFilteredRadiance->getCore();

		SPtr<ct::Texture> irradiance;
		if (mIrradiance)
			irradiance = mIrradiance->getCore();

		ct::Skybox* skybox = new (bs_alloc<ct::Skybox>()) ct::Skybox(filteredRadiance, irradiance);
		SPtr<ct::Skybox> skyboxPtr = bs_shared_ptr<ct::Skybox>(skybox);
		skyboxPtr->_setThisPtr(skyboxPtr);

		return skyboxPtr;
	}

	CoreSyncData Skybox::syncToCore(FrameAlloc* allocator)
	{
		UINT32 size = 0;
		size += rttiGetElemSize(mIsActive);
		size += rttiGetElemSize(mBrightness);
		size += sizeof(SPtr<ct::Texture>);
		size += rttiGetElemSize(getCoreDirtyFlags());

		UINT8* buffer = allocator->alloc(size);

		char* dataPtr = (char*)buffer;
		dataPtr = rttiWriteElem(mIsActive, dataPtr);
		dataPtr = rttiWriteElem(mBrightness, dataPtr);
		dataPtr = rttiWriteElem(getCoreDirtyFlags(), dataPtr);

		SPtr<ct::Texture>* texture = new (dataPtr) SPtr<ct::Texture>();
		if (mTexture.isLoaded(false))
			*texture = mTexture->getCore();
		else
			*texture = nullptr;

		dataPtr += sizeof(SPtr<ct::Texture>);

		return CoreSyncData(buffer, size);
	}

	void Skybox::_markCoreDirty(SkyboxDirtyFlag flags)
	{
		if(flags == SkyboxDirtyFlag::Texture)
			filterTexture();

		markCoreDirty((UINT32)flags);
	}

	RTTITypeBase* Skybox::getRTTIStatic()
	{
		return SkyboxRTTI::instance();
	}

	RTTITypeBase* Skybox::getRTTI() const
	{
		return Skybox::getRTTIStatic();
	}

	namespace ct
	{
		Skybox::Skybox(const SPtr<Texture>& filteredRadiance, const SPtr<Texture>& irradiance)
			:mFilteredRadiance(filteredRadiance), mIrradiance(irradiance)
		{ }

		Skybox::~Skybox()
		{
			gRenderer()->notifySkyboxRemoved(this);
		}

		void Skybox::initialize()
		{
			gRenderer()->notifySkyboxAdded(this);

			CoreObject::initialize();
		}

		void Skybox::syncToCore(const CoreSyncData& data)
		{
			char* dataPtr = (char*)data.getBuffer();

			SkyboxDirtyFlag dirtyFlags;
			bool oldIsActive = mIsActive;

			dataPtr = rttiReadElem(mIsActive, dataPtr);
			dataPtr = rttiReadElem(mBrightness, dataPtr);
			dataPtr = rttiReadElem(dirtyFlags, dataPtr);

			SPtr<Texture>* texture = (SPtr<Texture>*)dataPtr;

			mTexture = *texture;
			texture->~SPtr<Texture>();
			dataPtr += sizeof(SPtr<Texture>);

			if (oldIsActive != mIsActive)
			{
				if (mIsActive)
					gRenderer()->notifySkyboxAdded(this);
				else
					gRenderer()->notifySkyboxRemoved(this);
			}
			else
			{
				if (dirtyFlags != SkyboxDirtyFlag::Texture)
				{
					gRenderer()->notifySkyboxRemoved(this);
					gRenderer()->notifySkyboxAdded(this);
				}
			}
		}
	}
}