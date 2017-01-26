//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsLight.h"
#include "BsLightRTTI.h"
#include "BsRenderer.h"
#include "BsFrameAlloc.h"
#include "BsSceneObject.h"
#include "BsMesh.h"
#include "BsRendererUtility.h"

namespace bs
{
	LightBase::LightBase()
		: mPosition(BsZero), mRotation(BsIdentity), mType(LightType::Point), mCastsShadows(false), mColor(Color::White)
		, mRange(10.0f), mIntensity(5.0f), mSpotAngle(45), mSpotFalloffAngle(35.0f), mIsActive(true), mPhysCorrectAtten(true)
	{
		updatePhysicallyCorrectRange();
	}

	LightBase::LightBase(LightType type, Color color,
		float intensity, float range, bool castsShadows, Degree spotAngle, Degree spotFalloffAngle)
		: mPosition(BsZero), mRotation(BsIdentity), mType(type), mCastsShadows(castsShadows), mColor(color), mRange(range)
		, mIntensity(intensity), mSpotAngle(spotAngle), mSpotFalloffAngle(spotFalloffAngle), mIsActive(true)
		, mPhysCorrectAtten(true)
	{
		updatePhysicallyCorrectRange();
	}

	void LightBase::setPhysicallyBasedAttenuation(bool enabled)
	{
		mPhysCorrectAtten = enabled; 

		if(enabled)
			updatePhysicallyCorrectRange();

		_markCoreDirty();
	}

	void LightBase::setRange(float range)
	{
		if (mPhysCorrectAtten)
			return;

		mRange = range; 
		_markCoreDirty(); 
		updateBounds();
	}

	void LightBase::setIntensity(float intensity)
	{
		mIntensity = intensity; 

		if (mPhysCorrectAtten)
			updatePhysicallyCorrectRange();

		_markCoreDirty(); 
	}

	float LightBase::getRadiance() const
	{
		switch (mType)
		{
		case LightType::Point:
			return mIntensity / (4 * Math::PI);
		case LightType::Spot:
		{
			float cosTotalAngle = Math::cos(mSpotAngle);
			float cosFalloffAngle = Math::cos(mSpotFalloffAngle);

			return mIntensity / (Math::TWO_PI * (1.0f - (cosFalloffAngle + cosTotalAngle) * 0.5f));
		}
		case LightType::Directional:
			return mIntensity;
		default:
			return 0.0f;
		}
	}

	void LightBase::updatePhysicallyCorrectRange()
	{
		// When lower than this attenuation light influence is assumed to be zero
		const float minAttenuation = 0.05f;
		mRange = sqrt(std::max(0.0f, mIntensity / minAttenuation - 1.0f));

		updateBounds();
	}

	void LightBase::updateBounds()
	{
		switch (mType)
		{
		case LightType::Directional:
			mBounds = Sphere(mPosition, std::numeric_limits<float>::infinity());
			break;
		case LightType::Point:
			mBounds = Sphere(mPosition, mRange);
			break;
		case LightType::Spot:
		{
			Degree angle = Math::clamp(mSpotAngle, Degree(-89), Degree(89));
			float coneRadius = Math::tan(angle) * mRange;

			float radius;
			Vector3 offset;
			if (coneRadius < mRange)
			{
				radius = (mRange * mRange + coneRadius * coneRadius) / (0.5f * mRange);
				offset = Vector3(0, 0, -(mRange - coneRadius));
			}
			else
			{
				radius = coneRadius;
				offset = Vector3::ZERO;
			}

			Vector3 center = mPosition + mRotation.rotate(offset);
			mBounds = Sphere(center, radius);
		}
			break;
		}
	}

	Light::Light()
		:mLastUpdateHash(0)
	{
		
	}

	Light::Light(LightType type, Color color,
		float intensity, float range, bool castsShadows, Degree spotAngle, Degree spotFalloffAngle)
		: LightBase(type, color, intensity, range, castsShadows, spotAngle, spotFalloffAngle),
		mLastUpdateHash(0)
	{
		// Calling virtual method is okay here because this is the most derived type
		updateBounds();
	}

	SPtr<ct::Light> Light::getCore() const
	{
		return std::static_pointer_cast<ct::Light>(mCoreSpecific);
	}

	SPtr<Light> Light::create(LightType type, Color color,
		float intensity, float range, bool castsShadows, Degree spotAngle, Degree spotFalloffAngle)
	{
		Light* handler = new (bs_alloc<Light>()) 
			Light(type, color, intensity, range, castsShadows, spotAngle, spotFalloffAngle);
		SPtr<Light> handlerPtr = bs_core_ptr<Light>(handler);
		handlerPtr->_setThisPtr(handlerPtr);
		handlerPtr->initialize();

		return handlerPtr;
	}

	SPtr<Light> Light::createEmpty()
	{
		Light* handler = new (bs_alloc<Light>()) Light();
		SPtr<Light> handlerPtr = bs_core_ptr<Light>(handler);
		handlerPtr->_setThisPtr(handlerPtr);

		return handlerPtr;
	}

	SPtr<ct::CoreObject> Light::createCore() const
	{
		ct::Light* handler = new (bs_alloc<ct::Light>())
			ct::Light(mType, mColor, mIntensity, mRange, mCastsShadows, mSpotAngle, mSpotFalloffAngle);
		SPtr<ct::Light> handlerPtr = bs_shared_ptr<ct::Light>(handler);
		handlerPtr->_setThisPtr(handlerPtr);

		return handlerPtr;
	}

	CoreSyncData Light::syncToCore(FrameAlloc* allocator)
	{
		UINT32 size = 0;
		size += rttiGetElemSize(mPosition);
		size += rttiGetElemSize(mRotation);
		size += rttiGetElemSize(mType);
		size += rttiGetElemSize(mCastsShadows);
		size += rttiGetElemSize(mColor);
		size += rttiGetElemSize(mRange);
		size += rttiGetElemSize(mIntensity);
		size += rttiGetElemSize(mSpotAngle);
		size += rttiGetElemSize(mSpotFalloffAngle);
		size += rttiGetElemSize(mPhysCorrectAtten);
		size += rttiGetElemSize(mIsActive);
		size += rttiGetElemSize(getCoreDirtyFlags());
		size += rttiGetElemSize(mBounds);

		UINT8* buffer = allocator->alloc(size);

		char* dataPtr = (char*)buffer;
		dataPtr = rttiWriteElem(mPosition, dataPtr);
		dataPtr = rttiWriteElem(mRotation, dataPtr);
		dataPtr = rttiWriteElem(mType, dataPtr);
		dataPtr = rttiWriteElem(mCastsShadows, dataPtr);
		dataPtr = rttiWriteElem(mColor, dataPtr);
		dataPtr = rttiWriteElem(mRange, dataPtr);
		dataPtr = rttiWriteElem(mIntensity, dataPtr);
		dataPtr = rttiWriteElem(mSpotAngle, dataPtr);
		dataPtr = rttiWriteElem(mSpotFalloffAngle, dataPtr);
		dataPtr = rttiWriteElem(mPhysCorrectAtten, dataPtr);
		dataPtr = rttiWriteElem(mIsActive, dataPtr);
		dataPtr = rttiWriteElem(getCoreDirtyFlags(), dataPtr);
		dataPtr = rttiWriteElem(mBounds, dataPtr);

		return CoreSyncData(buffer, size);
	}

	void Light::_updateTransform(const HSceneObject& parent)
	{
		UINT32 curHash = parent->getTransformHash();
		if (curHash != _getLastModifiedHash())
		{
			setPosition(parent->getWorldPosition());
			setRotation(parent->getWorldRotation());
			_setLastModifiedHash(curHash);
		}
	}

	void Light::_markCoreDirty(LightDirtyFlag flag)
	{
		markCoreDirty((UINT32)flag);
	}

	RTTITypeBase* Light::getRTTIStatic()
	{
		return LightRTTI::instance();
	}

	RTTITypeBase* Light::getRTTI() const
	{
		return Light::getRTTIStatic();
	}

	namespace ct
	{
	const UINT32 Light::LIGHT_CONE_NUM_SIDES = 20;
	const UINT32 Light::LIGHT_CONE_NUM_SLICES = 10;

	Light::Light(LightType type, Color color,
		float intensity, float range, bool castsShadows, Degree spotAngle, Degree spotFalloffAngle)
		:LightBase(type, color, intensity, range, castsShadows, spotAngle, spotFalloffAngle), mRendererId(0)
	{

	}

	Light::~Light()
	{
		gRenderer()->notifyLightRemoved(this);
	}

	void Light::initialize()
	{
		updateBounds();
		gRenderer()->notifyLightAdded(this);

		CoreObject::initialize();
	}

	void Light::syncToCore(const CoreSyncData& data)
	{
		char* dataPtr = (char*)data.getBuffer();

		UINT32 dirtyFlags = 0;
		bool oldIsActive = mIsActive;
		LightType oldType = mType;

		dataPtr = rttiReadElem(mPosition, dataPtr);
		dataPtr = rttiReadElem(mRotation, dataPtr);
		dataPtr = rttiReadElem(mType, dataPtr);
		dataPtr = rttiReadElem(mCastsShadows, dataPtr);
		dataPtr = rttiReadElem(mColor, dataPtr);
		dataPtr = rttiReadElem(mRange, dataPtr);
		dataPtr = rttiReadElem(mIntensity, dataPtr);
		dataPtr = rttiReadElem(mSpotAngle, dataPtr);
		dataPtr = rttiReadElem(mSpotFalloffAngle, dataPtr);
		dataPtr = rttiReadElem(mPhysCorrectAtten, dataPtr);
		dataPtr = rttiReadElem(mIsActive, dataPtr);
		dataPtr = rttiReadElem(dirtyFlags, dataPtr);
		dataPtr = rttiReadElem(mBounds, dataPtr);

		updateBounds();

		if (dirtyFlags == (UINT32)LightDirtyFlag::Transform)
		{
			if (mIsActive)
				gRenderer()->notifyLightUpdated(this);
		}
		else
		{
			if (oldIsActive != mIsActive)
			{
				if (mIsActive)
					gRenderer()->notifyLightAdded(this);
				else
				{
					LightType newType = mType;
					mType = oldType;
					gRenderer()->notifyLightRemoved(this);
					mType = newType;
				}
			}
			else
			{
				LightType newType = mType;
				mType = oldType;
				gRenderer()->notifyLightRemoved(this);
				mType = newType;

				gRenderer()->notifyLightAdded(this);
			}
		}
	}

	SPtr<Mesh> Light::getMesh() const
	{
		switch (mType)
		{
		case LightType::Directional:
			return nullptr;
		case LightType::Point:
			return RendererUtility::instance().getPointLightStencil();
		case LightType::Spot:
			return RendererUtility::instance().getSpotLightStencil();
		}

		return nullptr;
	}
	}
}