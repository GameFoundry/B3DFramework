//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCLight.h"
#include "Private/RTTI/BsCLightRTTI.h"
#include "Scene/BsSceneManager.h"

using namespace bs;

CLight::CLight()
{
	SetFlag(ComponentFlag::AlwaysRun, true);
	SetName("Light");
}

CLight::CLight(const HSceneObject& parent, LightType type, Color color, float intensity, float range, bool castsShadows, Degree spotAngle, Degree spotFalloffAngle)
	: Component(parent), mType(type), mColor(color), mIntensity(intensity), mRange(range), mCastsShadows(castsShadows), mSpotAngle(spotAngle), mSpotFalloffAngle(spotFalloffAngle)
{
	SetFlag(ComponentFlag::AlwaysRun, true);
	SetName("Light");
}

CLight::~CLight()
{}

Sphere CLight::GetBounds() const
{
	mInternal->UpdateStateInternal(*SO());

	return mInternal->GetBounds();
}

void CLight::OnBeginPlay()
{
	// If mInternal already exists this means this object was deserialized,
	// so all we need to do is initialize it.
	if(mInternal != nullptr)
		mInternal->Initialize();
	else
	{
		mInternal = Light::Create(
			mType,
			mColor,
			mIntensity,
			mRange,
			mCastsShadows,
			mSpotAngle,
			mSpotFalloffAngle);
	}

	GetSceneManager().BindActorInternal(mInternal, SceneObject());
}

void CLight::OnDestroyed()
{
	GetSceneManager().UnbindActorInternal(mInternal);
	mInternal->Destroy();
}

RTTITypeBase* CLight::GetRttiStatic()
{
	return CLightRTTI::Instance();
}

RTTITypeBase* CLight::GetRtti() const
{
	return CLight::GetRttiStatic();
}
