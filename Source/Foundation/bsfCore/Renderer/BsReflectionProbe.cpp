//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsReflectionProbe.h"
#include "Private/RTTI/BsReflectionProbeRTTI.h"
#include "RTTI/BsMathRTTI.h"
#include "Scene/BsSceneObject.h"
#include "Image/BsTexture.h"
#include "Renderer/BsRenderer.h"
#include "Utility/BsUUID.h"
#include "Renderer/BsIBLUtility.h"
#include "CoreThread/BsCoreObjectSync.h"

namespace bs
{
	ReflectionProbeBase::ReflectionProbeBase(ReflectionProbeType type, float radius, const Vector3& extents)
		: mType(type), mRadius(radius), mExtents(extents)
	{ }

	float ReflectionProbeBase::getRadius() const
	{
		Vector3 scale = mTransform.getScale();
		return mRadius * std::max(std::max(scale.x, scale.y), scale.z);
	}

	void ReflectionProbeBase::updateBounds()
	{
		Vector3 position = mTransform.getPosition();
		Vector3 scale = mTransform.getScale();

		switch (mType)
		{
		case ReflectionProbeType::Sphere:
			mBounds = Sphere(position, mRadius * std::max(std::max(scale.x, scale.y), scale.z));
			break;
		case ReflectionProbeType::Box:
			mBounds = Sphere(position, (mExtents * scale).length());
			break;
		}
	}

	template <bool Core>
	template <class P>
	void TReflectionProbe<Core>::rttiEnumFields(P p)
	{
		p(mType);
		p(mRadius);
		p(mExtents);
		p(mTransitionDistance);
		p(mBounds);
		p(mFilteredTexture);
	}

	ReflectionProbe::ReflectionProbe(ReflectionProbeType type, float radius, const Vector3& extents)
		: TReflectionProbe(type, radius, extents)
	{
		// Calling virtual method is okay here because this is the most derived type
		updateBounds();
	}

	ReflectionProbe::~ReflectionProbe()
	{
		if (mRendererTask)
			mRendererTask->cancel();
	}

	void ReflectionProbe::capture()
	{
		if (mCustomTexture != nullptr)
			return;

		captureAndFilter();
	}

	void ReflectionProbe::filter()
	{
		if (mCustomTexture == nullptr)
			return;

		captureAndFilter();
	}

	void ReflectionProbe::captureAndFilter()
	{
		// If previous rendering task exists, cancel it
		if (mRendererTask != nullptr)
			mRendererTask->cancel();

		TEXTURE_DESC cubemapDesc;
		cubemapDesc.type = TEX_TYPE_CUBE_MAP;
		cubemapDesc.format = PF_RG11B10F;
		cubemapDesc.width = ct::IBLUtility::REFLECTION_CUBEMAP_SIZE;
		cubemapDesc.height = ct::IBLUtility::REFLECTION_CUBEMAP_SIZE;
		cubemapDesc.numMips = PixelUtil::getMaxMipmaps(cubemapDesc.width, cubemapDesc.height, 1, cubemapDesc.format);
		cubemapDesc.usage = TU_STATIC | TU_RENDERTARGET;

		mFilteredTexture = Texture::_createPtr(cubemapDesc);

		auto renderComplete = [this]()
		{
			mRendererTask = nullptr;
		};

		SPtr<ct::ReflectionProbe> coreProbe = getCore();
		SPtr<ct::Texture> coreTexture = mFilteredTexture->getCore();

		if (mCustomTexture == nullptr)
		{
			auto renderReflProbe = [coreTexture, coreProbe]()
			{
				float radius = coreProbe->mType == ReflectionProbeType::Sphere ? coreProbe->mRadius :
					coreProbe->mExtents.length();

				ct::CaptureSettings settings;
				settings.encodeDepth = true;
				settings.depthEncodeNear = radius;
				settings.depthEncodeFar = radius + 1; // + 1 arbitrary, make it a customizable value?

				ct::gRenderer()->captureSceneCubeMap(coreTexture, coreProbe->getTransform().getPosition(), settings);
				ct::gIBLUtility().filterCubemapForSpecular(coreTexture, nullptr);

				coreProbe->mFilteredTexture = coreTexture;
				ct::gRenderer()->notifyReflectionProbeUpdated(coreProbe.get(), true);

				return true;
			};

			mRendererTask = ct::RendererTask::create("ReflProbeRender", renderReflProbe);
		}
		else
		{
			SPtr<ct::Texture> coreCustomTex = mCustomTexture->getCore();
			auto filterReflProbe = [coreCustomTex, coreTexture, coreProbe]()
			{
				ct::gIBLUtility().scaleCubemap(coreCustomTex, 0, coreTexture, 0);
				ct::gIBLUtility().filterCubemapForSpecular(coreTexture, nullptr);

				coreProbe->mFilteredTexture = coreTexture;
				ct::gRenderer()->notifyReflectionProbeUpdated(coreProbe.get(), true);

				return true;
			};

			mRendererTask = ct::RendererTask::create("ReflProbeRender", filterReflProbe);
		}

		mRendererTask->onComplete.connect(renderComplete);
		ct::gRenderer()->addTask(mRendererTask);
	}

	SPtr<ct::ReflectionProbe> ReflectionProbe::getCore() const
	{
		return std::static_pointer_cast<ct::ReflectionProbe>(mCoreSpecific);
	}

	SPtr<ReflectionProbe> ReflectionProbe::createSphere(float radius)
	{
		ReflectionProbe* probe = new (bs_alloc<ReflectionProbe>()) ReflectionProbe(ReflectionProbeType::Sphere, radius, Vector3::ZERO);
		SPtr<ReflectionProbe> probePtr = bs_core_ptr<ReflectionProbe>(probe);
		probePtr->_setThisPtr(probePtr);
		probePtr->initialize();

		return probePtr;
	}

	SPtr<ReflectionProbe> ReflectionProbe::createBox(const Vector3& extents)
	{
		ReflectionProbe* probe = new (bs_alloc<ReflectionProbe>()) ReflectionProbe(ReflectionProbeType::Box, 1.0f, extents);
		SPtr<ReflectionProbe> probePtr = bs_core_ptr<ReflectionProbe>(probe);
		probePtr->_setThisPtr(probePtr);
		probePtr->initialize();

		return probePtr;
	}

	SPtr<ReflectionProbe> ReflectionProbe::createEmpty()
	{
		ReflectionProbe* probe = new (bs_alloc<ReflectionProbe>()) ReflectionProbe();
		SPtr<ReflectionProbe> probePtr = bs_core_ptr<ReflectionProbe>(probe);
		probePtr->_setThisPtr(probePtr);

		return probePtr;
	}

	SPtr<ct::CoreObject> ReflectionProbe::createCore() const
	{
		SPtr<ct::Texture> filteredTexture;
		if (mFilteredTexture != nullptr)
			filteredTexture = mFilteredTexture->getCore();

		ct::ReflectionProbe* probe = new (bs_alloc<ct::ReflectionProbe>()) ct::ReflectionProbe(mType, mRadius, mExtents,
			filteredTexture);
		SPtr<ct::ReflectionProbe> probePtr = bs_shared_ptr<ct::ReflectionProbe>(probe);
		probePtr->_setThisPtr(probePtr);

		return probePtr;
	}

	CoreSyncData ReflectionProbe::syncToCore(FrameAlloc* allocator)
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

	void ReflectionProbe::_markCoreDirty(ActorDirtyFlag flags)
	{
		markCoreDirty((UINT32)flags);
	}

	RTTITypeBase* ReflectionProbe::getRTTIStatic()
	{
		return ReflectionProbeRTTI::instance();
	}

	RTTITypeBase* ReflectionProbe::getRTTI() const
	{
		return ReflectionProbe::getRTTIStatic();
	}

	template class TReflectionProbe<true>;
	template class TReflectionProbe<false>;

	namespace ct
	{
	ReflectionProbe::ReflectionProbe(ReflectionProbeType type, float radius, const Vector3& extents,
		const SPtr<Texture>& filteredTexture)
		: TReflectionProbe(type, radius, extents), mRendererId(0)
	{
		mFilteredTexture = filteredTexture;
	}

	ReflectionProbe::~ReflectionProbe()
	{
		gRenderer()->notifyReflectionProbeRemoved(this);
	}

	void ReflectionProbe::initialize()
	{
		updateBounds();
		gRenderer()->notifyReflectionProbeAdded(this);

		CoreObject::initialize();
	}

	void ReflectionProbe::syncToCore(const CoreSyncData& data)
	{
		Bitstream stream(data.getBuffer(), data.getBufferSize());

		UINT32 dirtyFlags = 0;
		bool oldIsActive = mActive;
		ReflectionProbeType oldType = mType;

		rtti_read(dirtyFlags, stream);
		csync_read((SceneActor&)*this, stream);
		csync_read(*this, stream);

		updateBounds();

		if (dirtyFlags == (UINT32)ActorDirtyFlag::Transform)
		{
			if (mActive)
				gRenderer()->notifyReflectionProbeUpdated(this, false);
		}
		else
		{
			if (oldIsActive != mActive)
			{
				if (mActive)
					gRenderer()->notifyReflectionProbeAdded(this);
				else
				{
					ReflectionProbeType newType = mType;
					mType = oldType;
					gRenderer()->notifyReflectionProbeRemoved(this);
					mType = newType;
				}
			}
			else
			{
				ReflectionProbeType newType = mType;
				mType = oldType;
				gRenderer()->notifyReflectionProbeRemoved(this);
				mType = newType;

				gRenderer()->notifyReflectionProbeAdded(this);
			}
		}
	}
}}
