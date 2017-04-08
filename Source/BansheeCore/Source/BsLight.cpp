//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsLight.h"
#include "BsLightRTTI.h"
#include "BsRenderer.h"
#include "BsFrameAlloc.h"
#include "BsSceneObject.h"
#include "BsMesh.h"

namespace bs
{
	LightBase::LightBase()
		: mPosition(BsZero), mRotation(BsIdentity), mType(LightType::Radial), mCastsShadows(false), mColor(Color::White)
		, mAttRadius(10.0f), mSourceRadius(0.0f), mIntensity(5.0f), mSpotAngle(45), mSpotFalloffAngle(35.0f)
		, mIsActive(true), mAutoAttenuation(true)
	{
		updateAttenuationRange();
	}

	LightBase::LightBase(LightType type, Color color, float intensity, float attRadius, float srcRadius, bool castsShadows, 
		Degree spotAngle, Degree spotFalloffAngle)
		: mPosition(BsZero), mRotation(BsIdentity), mType(type), mCastsShadows(castsShadows), mColor(color)
		, mAttRadius(attRadius), mSourceRadius(srcRadius), mIntensity(intensity), mSpotAngle(spotAngle)
		, mSpotFalloffAngle(spotFalloffAngle), mIsActive(true), mAutoAttenuation(true)
	{
		updateAttenuationRange();
	}

	void LightBase::setUseAutoAttenuation(bool enabled)
	{
		mAutoAttenuation = enabled; 

		if(enabled)
			updateAttenuationRange();

		_markCoreDirty();
	}

	void LightBase::setAttenuationRadius(float radius)
	{
		if (mAutoAttenuation)
			return;

		mAttRadius = radius;
		_markCoreDirty(); 
		updateBounds();
	}

	void LightBase::setSourceRadius(float radius)
	{
		mSourceRadius = radius;

		if (mAutoAttenuation)
			updateAttenuationRange();

		_markCoreDirty();
	}

	void LightBase::setIntensity(float intensity)
	{
		mIntensity = intensity; 

		if (mAutoAttenuation)
			updateAttenuationRange();

		_markCoreDirty(); 
	}

	float LightBase::getLuminance() const
	{
		float radius2 = mSourceRadius * mSourceRadius;

		switch (mType)
		{
		case LightType::Radial:
			if (mSourceRadius > 0.0f)
				return mIntensity / (4 * radius2 * Math::PI); // Luminous flux -> luminance
			else
				return mIntensity / (4 * Math::PI); // Luminous flux -> luminous intensity
		case LightType::Spot:
		{
			if (mSourceRadius > 0.0f) 
				return mIntensity / (radius2 * Math::PI); // Luminous flux -> luminance
			else 
			{
				// Note: Consider using the simpler conversion I / PI to match with the area-light conversion
				float cosTotalAngle = Math::cos(mSpotAngle);
				float cosFalloffAngle = Math::cos(mSpotFalloffAngle);

				// Luminous flux -> luminous intensity
				return mIntensity / (Math::TWO_PI * (1.0f - (cosFalloffAngle + cosTotalAngle) * 0.5f));
			}
		}
		case LightType::Directional:
			if (mSourceRadius > 0.0f)
			{
				// Use cone solid angle formulae to calculate disc solid angle
				float solidAngle = Math::TWO_PI * (1 - cos(mSourceRadius * Math::DEG2RAD));
				return mIntensity / solidAngle; // Illuminance -> luminance
			}
			else
				return mIntensity; // In luminance units by default
		default:
			return 0.0f;
		}
	}

	void LightBase::updateAttenuationRange()
	{
		// Value to which intensity needs to drop in order for the light contribution to fade out to zero
		const float minAttenuation = 0.2f;

		if(mSourceRadius > 0.0f)
		{
			// Inverse of the attenuation formula for area lights:
			//   a = I / (1 + (2/r) * d + (1/r^2) * d^2
			// Where r is the source radius, and d is the distance from the light. As derived here:
			//   https://imdoingitwrong.wordpress.com/2011/01/31/light-attenuation/

			float luminousFlux = getIntensity();

			float a = sqrt(minAttenuation);
			mAttRadius = (mSourceRadius * (sqrt(luminousFlux - a))) / a;
		}
		else // Based on the basic inverse square distance formula
		{
			float luminousIntensity = getIntensity();

			float a = minAttenuation;
			mAttRadius = sqrt(std::max(0.0f, luminousIntensity / a));
		}

		updateBounds();
	}

	void LightBase::updateBounds()
	{
		switch (mType)
		{
		case LightType::Directional:
			mBounds = Sphere(mPosition, std::numeric_limits<float>::infinity());
			break;
		case LightType::Radial:
			mBounds = Sphere(mPosition, mAttRadius);
			break;
		case LightType::Spot:
		{
			// Note: We could use the formula for calculating the circumcircle of a triangle (after projecting the cone),
			// but the radius of the sphere is the same as in the formula we use here, yet it is much simpler with no need
			// to calculate multiple determinants. Neither are good approximations when cone angle is wide.
			Vector3 offset(0, 0, mAttRadius * 0.5f);

			// Direction along the edge of the cone, on the YZ plane (doesn't matter if we used XZ instead)
			Degree angle = Math::clamp(mSpotAngle, Degree(-89), Degree(89));
			Vector3 coneDir(0, Math::tan(angle)*mAttRadius, mAttRadius);

			// Distance between the "corner" of the cone and our center, must be the radius (provided the center is at
			// the middle of the range)
			float radius = (offset - coneDir).length();

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

	Light::Light(LightType type, Color color, float intensity, float attRadius, float srcRadius, bool castsShadows, 
		Degree spotAngle, Degree spotFalloffAngle)
		: LightBase(type, color, intensity, attRadius, srcRadius, castsShadows, spotAngle, spotFalloffAngle),
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
		float intensity, float attRadius, bool castsShadows, Degree spotAngle, Degree spotFalloffAngle)
	{
		Light* handler = new (bs_alloc<Light>()) 
			Light(type, color, intensity, attRadius, 0.0f, castsShadows, spotAngle, spotFalloffAngle);
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
			ct::Light(mType, mColor, mIntensity, mAttRadius, mSourceRadius, mCastsShadows, mSpotAngle, mSpotFalloffAngle);
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
		size += rttiGetElemSize(mAttRadius);
		size += rttiGetElemSize(mSourceRadius);
		size += rttiGetElemSize(mIntensity);
		size += rttiGetElemSize(mSpotAngle);
		size += rttiGetElemSize(mSpotFalloffAngle);
		size += rttiGetElemSize(mAutoAttenuation);
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
		dataPtr = rttiWriteElem(mAttRadius, dataPtr);
		dataPtr = rttiWriteElem(mSourceRadius, dataPtr);
		dataPtr = rttiWriteElem(mIntensity, dataPtr);
		dataPtr = rttiWriteElem(mSpotAngle, dataPtr);
		dataPtr = rttiWriteElem(mSpotFalloffAngle, dataPtr);
		dataPtr = rttiWriteElem(mAutoAttenuation, dataPtr);
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
		float intensity, float attRadius, float srcRadius, bool castsShadows, Degree spotAngle, Degree spotFalloffAngle)
		:LightBase(type, color, intensity, attRadius, srcRadius, castsShadows, spotAngle, spotFalloffAngle), mRendererId(0)
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
		dataPtr = rttiReadElem(mAttRadius, dataPtr);
		dataPtr = rttiReadElem(mSourceRadius, dataPtr);
		dataPtr = rttiReadElem(mIntensity, dataPtr);
		dataPtr = rttiReadElem(mSpotAngle, dataPtr);
		dataPtr = rttiReadElem(mSpotFalloffAngle, dataPtr);
		dataPtr = rttiReadElem(mAutoAttenuation, dataPtr);
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

}}