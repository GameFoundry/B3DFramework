//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsSkybox.h"
#include "Private/RTTI/BsSkyboxRTTI.h"
#include "Scene/BsSceneObject.h"
#include "Image/BsTexture.h"
#include "Renderer/BsRenderer.h"
#include "Utility/BsUUID.h"
#include "Renderer/BsIBLUtility.h"
#include "CoreThread/BsCoreObjectSync.h"

namespace bs
{
	template <bool Core>
	template <class P>
	void TSkybox<Core>::rttiEnumFields(P p)
	{
		p(mBrightness);
		p(mTexture);
	}

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
			ct::gIBLUtility().filterCubemapForIrradiance(coreSkybox->getTexture(), coreIrradiance);
			coreSkybox->mIrradiance = coreIrradiance;

			return true;
		};

		mRendererTask = ct::RendererTask::create("SkyboxFilter", filterSkybox);

		mRendererTask->onComplete.connect(renderComplete);
		ct::gRenderer()->addTask(mRendererTask);
	}

	void Skybox::setTexture(const HTexture& texture)
	{
		mTexture = texture;

		mFilteredRadiance = nullptr;
		mIrradiance = nullptr;

		if(mTexture.isLoaded())
			filterTexture();

		_markCoreDirty((ActorDirtyFlag)SkyboxDirtyFlag::Texture);
	}

	SPtr<ct::Skybox> Skybox::getCore() const
	{
		return std::static_pointer_cast<ct::Skybox>(mCoreSpecific);
	}

	SPtr<Skybox> Skybox::createEmpty()
	{
		Skybox* skybox = new (bs_alloc<Skybox>()) Skybox();
		SPtr<Skybox> skyboxPtr = bs_core_ptr<Skybox>(skybox);
		skyboxPtr->_setThisPtr(skyboxPtr);

		return skyboxPtr;
	}

	SPtr<Skybox> Skybox::create()
	{
		SPtr<Skybox> skyboxPtr = createEmpty();
		skyboxPtr->initialize();

		return skyboxPtr;
	}

	SPtr<ct::CoreObject> Skybox::createCore() const
	{
		SPtr<ct::Texture> radiance;
		if (mTexture.isLoaded(false))
			radiance = mTexture->getCore();

		SPtr<ct::Texture> filteredRadiance;
		if (mFilteredRadiance)
			filteredRadiance = mFilteredRadiance->getCore();

		SPtr<ct::Texture> irradiance;
		if (mIrradiance)
			irradiance = mIrradiance->getCore();

		ct::Skybox* skybox = new (bs_alloc<ct::Skybox>()) ct::Skybox(radiance, filteredRadiance, irradiance);
		SPtr<ct::Skybox> skyboxPtr = bs_shared_ptr<ct::Skybox>(skybox);
		skyboxPtr->_setThisPtr(skyboxPtr);

		return skyboxPtr;
	}

	CoreSyncData Skybox::syncToCore(FrameAlloc* allocator)
	{
		UINT32 size = 0;
		size += rtti_size(getCoreDirtyFlags()).bytes;
		size += csync_size((SceneActor&)*this);
		size += csync_size(*this);

		UINT8* buffer = allocator->alloc(size);

		Bitstream stream(buffer, size);
		rtti_write(getCoreDirtyFlags(), stream);
		csync_write((SceneActor&)*this, stream);
		csync_write(*this, stream);

		return CoreSyncData(buffer, size);
	}

	void Skybox::_markCoreDirty(ActorDirtyFlag flags)
	{
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
		Skybox::Skybox(const SPtr<Texture>& radiance, const SPtr<Texture>& filteredRadiance, const SPtr<Texture>& irradiance)
			:mFilteredRadiance(filteredRadiance), mIrradiance(irradiance)
		{
			mTexture = radiance;
		}

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
			Bitstream stream(data.getBuffer(), data.getBufferSize());

			SkyboxDirtyFlag dirtyFlags;
			bool oldIsActive = mActive;

			rtti_read(dirtyFlags, stream);
			csync_read((SceneActor&)*this, stream);
			csync_read(*this, stream);

			if (oldIsActive != mActive)
			{
				if (mActive)
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
