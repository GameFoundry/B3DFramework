//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCLight.h"
#include "Private/RTTI/BsCLightRTTI.h"
#include "Scene/BsSceneManager.h"

namespace bs
{
	CLight::CLight()
	{
		setFlag(ComponentFlag::AlwaysRun, true);
		setName("Light");
	}

	CLight::CLight(const HSceneObject& parent, LightType type, Color color,
		float intensity, float range, bool castsShadows, Degree spotAngle, Degree spotFalloffAngle)
		: Component(parent), mType(type), mColor(color), mIntensity(intensity), mRange(range),
		mCastsShadows(castsShadows), mSpotAngle(spotAngle), mSpotFalloffAngle(spotFalloffAngle)
	{
		setFlag(ComponentFlag::AlwaysRun, true);
		setName("Light");
	}

	CLight::~CLight()
	{ }

	Sphere CLight::getBounds() const
	{
		mInternal->UpdateStateInternal(*SO());

		return mInternal->getBounds();
	}

	void CLight::onInitialized()
	{
		// If mInternal already exists this means this object was deserialized,
		// so all we need to do is initialize it.
		if (mInternal != nullptr)
			mInternal->initialize();
		else
		{
			mInternal = Light::create(
				mType,
				mColor,
				mIntensity,
				mRange,
				mCastsShadows,
				mSpotAngle,
				mSpotFalloffAngle);
		}

		gSceneManager().BindActorInternal(mInternal, sceneObject());
	}

	void CLight::onDestroyed()
	{
		gSceneManager().UnbindActorInternal(mInternal);
		mInternal->destroy();
	}
	
	RTTITypeBase* CLight::getRTTIStatic()
	{
		return CLightRTTI::instance();
	}

	RTTITypeBase* CLight::getRTTI() const
	{
		return CLight::getRTTIStatic();
	}
}
