//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Renderer/BsReflectionProbe.h"
#include "RTTI/BsReflectionProbeRTTI.h"
#include "Scene/BsSceneObject.h"
#include "Allocators/BsFrameAlloc.h"
#include "Image/BsTexture.h"
#include "Renderer/BsRenderer.h"
#include "Utility/BsUUID.h"
#include "Renderer/BsIBLUtility.h"

namespace bs
{
	ReflectionProbeBase::ReflectionProbeBase()
		: mType(ReflectionProbeType::Box), mRadius(1.0f), mExtents(1.0f, 1.0f, 1.0f), mTransitionDistance(0.1f)
		, mBounds(Vector3::ZERO, 1.0f)
	{ }

	ReflectionProbeBase::ReflectionProbeBase(ReflectionProbeType type, float radius, const Vector3& extents)
		: mType(type), mRadius(radius), mExtents(extents), mTransitionDistance(0.1f), mBounds(Vector3::ZERO, 1.0f)
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

	ReflectionProbe::ReflectionProbe()
	{

	}

	ReflectionProbe::ReflectionProbe(ReflectionProbeType type, float radius, const Vector3& extents)
		: ReflectionProbeBase(type, radius, extents)
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
		ReflectionProbe* probe = new (bs_alloc<ReflectionProbe>()) ReflectionProbe(ReflectionProbeType::Box, 0.0f, extents);
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
		UINT32 size = getActorSyncDataSize();
		size += rttiGetElemSize(mType);
		size += rttiGetElemSize(mRadius);
		size += rttiGetElemSize(mExtents);
		size += rttiGetElemSize(mTransitionDistance);
		size += rttiGetElemSize(getCoreDirtyFlags());
		size += rttiGetElemSize(mBounds);
		size += sizeof(SPtr<ct::Texture>);

		UINT8* buffer = allocator->alloc(size);

		char* dataPtr = (char*)buffer;
		dataPtr = syncActorTo(dataPtr);
		dataPtr = rttiWriteElem(mType, dataPtr);
		dataPtr = rttiWriteElem(mRadius, dataPtr);
		dataPtr = rttiWriteElem(mExtents, dataPtr);
		dataPtr = rttiWriteElem(mTransitionDistance, dataPtr);
		dataPtr = rttiWriteElem(getCoreDirtyFlags(), dataPtr);
		dataPtr = rttiWriteElem(mBounds, dataPtr);

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

	namespace ct
	{
	ReflectionProbe::ReflectionProbe(ReflectionProbeType type, float radius, const Vector3& extents, 
		const SPtr<Texture>& filteredTexture)
		: ReflectionProbeBase(type, radius, extents), mRendererId(0), mFilteredTexture(filteredTexture)
	{

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
		char* dataPtr = (char*)data.getBuffer();

		UINT32 dirtyFlags = 0;
		bool oldIsActive = mActive;
		ReflectionProbeType oldType = mType;

		dataPtr = syncActorFrom(dataPtr);
		dataPtr = rttiReadElem(mType, dataPtr);
		dataPtr = rttiReadElem(mRadius, dataPtr);
		dataPtr = rttiReadElem(mExtents, dataPtr);
		dataPtr = rttiReadElem(mTransitionDistance, dataPtr);
		dataPtr = rttiReadElem(dirtyFlags, dataPtr);
		dataPtr = rttiReadElem(mBounds, dataPtr);

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
