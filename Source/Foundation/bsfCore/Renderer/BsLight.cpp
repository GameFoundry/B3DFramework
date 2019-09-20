//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsLight.h"
#include "Private/RTTI/BsLightRTTI.h"
#include "Renderer/BsRenderer.h"
#include "Scene/BsSceneObject.h"
#include "Mesh/BsMesh.h"
#include "CoreThread/BsCoreObjectSync.h"

namespace bs
{
	LightBase::LightBase()
		: mType(LightType::Radial), mCastsShadows(false), mColor(Color::White), mAttRadius(10.0f), mSourceRadius(0.0f)
		, mIntensity(100.0f), mSpotAngle(45), mSpotFalloffAngle(35.0f), mAutoAttenuation(false), mShadowBias(0.5f)
	{
		updateAttenuationRange();
	}

	LightBase::LightBase(LightType type, Color color, float intensity, float attRadius, float srcRadius, bool castsShadows,
		Degree spotAngle, Degree spotFalloffAngle)
		: mType(type), mCastsShadows(castsShadows), mColor(color), mAttRadius(attRadius), mSourceRadius(srcRadius)
		, mIntensity(intensity), mSpotAngle(spotAngle), mSpotFalloffAngle(spotFalloffAngle), mAutoAttenuation(false)
		, mShadowBias(0.5f)
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
		const Transform& tfrm = getTransform();

		switch (mType)
		{
		case LightType::Directional:
			mBounds = Sphere(tfrm.getPosition(), std::numeric_limits<float>::infinity());
			break;
		case LightType::Radial:
			mBounds = Sphere(tfrm.getPosition(), mAttRadius);
			break;
		case LightType::Spot:
		{
			// Note: We could use the formula for calculating the circumcircle of a triangle (after projecting the cone),
			// but the radius of the sphere is the same as in the formula we use here, yet it is much simpler with no need
			// to calculate multiple determinants. Neither are good approximations when cone angle is wide.
			Vector3 offset(0, 0, mAttRadius * 0.5f);

			// Direction along the edge of the cone, on the YZ plane (doesn't matter if we used XZ instead)
			Degree angle = Math::clamp(mSpotAngle * 0.5f, Degree(-89), Degree(89));
			Vector3 coneDir(0, Math::tan(angle)*mAttRadius, mAttRadius);

			// Distance between the "corner" of the cone and our center, must be the radius (provided the center is at
			// the middle of the range)
			float radius = (offset - coneDir).length();

			Vector3 center = tfrm.getPosition() - tfrm.getRotation().rotate(offset);
			mBounds = Sphere(center, radius);
		}
			break;
		default:
			break;
		}
	}

	void LightBase::setTransform(const Transform& transform)
	{
		if (mMobility != ObjectMobility::Movable)
			return;

		SceneActor::setTransform(transform);
		updateBounds();
	}

	template <class P>
	void LightBase::rttiEnumFields(P p)
	{
		p(mType);
		p(mCastsShadows);
		p(mColor);
		p(mAttRadius);
		p(mSourceRadius);
		p(mIntensity);
		p(mSpotAngle);
		p(mSpotFalloffAngle);
		p(mAutoAttenuation);
		p(mBounds);
		p(mShadowBias);
	}

	Light::Light(LightType type, Color color, float intensity, float attRadius, float srcRadius, bool castsShadows,
		Degree spotAngle, Degree spotFalloffAngle)
		: LightBase(type, color, intensity, attRadius, srcRadius, castsShadows, spotAngle, spotFalloffAngle)
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

	void Light::_markCoreDirty(ActorDirtyFlag flag)
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
		Bitstream stream(data.getBuffer(), data.getBufferSize());

		UINT32 dirtyFlags = 0;
		bool oldIsActive = mActive;
		LightType oldType = mType;

		rtti_read(dirtyFlags, stream);
		csync_read((SceneActor&)*this, stream);
		csync_read(*this, stream);

		updateBounds();

		if((dirtyFlags & ((UINT32)ActorDirtyFlag::Everything | (UINT32)ActorDirtyFlag::Active)) != 0)
		{
			if (oldIsActive != mActive)
			{
				if (mActive)
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
		else if((dirtyFlags & (UINT32)ActorDirtyFlag::Mobility) != 0)
		{
			gRenderer()->notifyLightRemoved(this);
			gRenderer()->notifyLightAdded(this);
		}
		else if ((dirtyFlags & (UINT32)ActorDirtyFlag::Transform) != 0)
		{
			if (mActive)
				gRenderer()->notifyLightUpdated(this);
		}
	}
}}
