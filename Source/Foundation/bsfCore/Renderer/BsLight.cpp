//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsLight.h"
#include "Private/RTTI/BsLightRTTI.h"
#include "Renderer/BsRenderer.h"
#include "Scene/BsSceneObject.h"
#include "Mesh/BsMesh.h"
#include "CoreObject/BsCoreObjectSync.h"

using namespace bs;

LightBase::LightBase()
	: mType(LightType::Radial), mCastsShadows(false), mColor(Color::kWhite), mAttRadius(10.0f), mSourceRadius(0.0f), mIntensity(100.0f), mSpotAngle(45), mSpotFalloffAngle(35.0f), mAutoAttenuation(false), mShadowBias(0.5f)
{
	UpdateAttenuationRange();
}

LightBase::LightBase(LightType type, Color color, float intensity, float attRadius, float srcRadius, bool castsShadows, Degree spotAngle, Degree spotFalloffAngle)
	: mType(type), mCastsShadows(castsShadows), mColor(color), mAttRadius(attRadius), mSourceRadius(srcRadius), mIntensity(intensity), mSpotAngle(spotAngle), mSpotFalloffAngle(spotFalloffAngle), mAutoAttenuation(false), mShadowBias(0.5f)
{
	UpdateAttenuationRange();
}

void LightBase::SetUseAutoAttenuation(bool enabled)
{
	mAutoAttenuation = enabled;

	if(enabled)
		UpdateAttenuationRange();

	MarkRenderProxyDataDirtyInternal();
}

void LightBase::SetAttenuationRadius(float radius)
{
	if(mAutoAttenuation)
		return;

	mAttRadius = radius;
	MarkRenderProxyDataDirtyInternal();
	UpdateBounds();
}

void LightBase::SetSourceRadius(float radius)
{
	mSourceRadius = radius;

	if(mAutoAttenuation)
		UpdateAttenuationRange();

	MarkRenderProxyDataDirtyInternal();
}

void LightBase::SetIntensity(float intensity)
{
	mIntensity = intensity;

	if(mAutoAttenuation)
		UpdateAttenuationRange();

	MarkRenderProxyDataDirtyInternal();
}

float LightBase::GetLuminance() const
{
	float radius2 = mSourceRadius * mSourceRadius;

	switch(mType)
	{
	case LightType::Radial:
		if(mSourceRadius > 0.0f)
			return mIntensity / (4 * radius2 * Math::kPi); // Luminous flux -> luminance
		else
			return mIntensity / (4 * Math::kPi); // Luminous flux -> luminous intensity
	case LightType::Spot:
		{
			if(mSourceRadius > 0.0f)
				return mIntensity / (radius2 * Math::kPi); // Luminous flux -> luminance
			else
			{
				// Note: Consider using the simpler conversion I / PI to match with the area-light conversion
				float cosTotalAngle = Math::Cos(mSpotAngle);
				float cosFalloffAngle = Math::Cos(mSpotFalloffAngle);

				// Luminous flux -> luminous intensity
				return mIntensity / (Math::kTwoPi * (1.0f - (cosFalloffAngle + cosTotalAngle) * 0.5f));
			}
		}
	case LightType::Directional:
		if(mSourceRadius > 0.0f)
		{
			// Use cone solid angle formulae to calculate disc solid angle
			float solidAngle = Math::kTwoPi * (1 - cos(mSourceRadius * Math::kDeG2Rad));
			return mIntensity / solidAngle; // Illuminance -> luminance
		}
		else
			return mIntensity; // In luminance units by default
	default:
		return 0.0f;
	}
}

void LightBase::UpdateAttenuationRange()
{
	// Value to which intensity needs to drop in order for the light contribution to fade out to zero
	const float minAttenuation = 0.2f;

	if(mSourceRadius > 0.0f)
	{
		// Inverse of the attenuation formula for area lights:
		//   a = I / (1 + (2/r) * d + (1/r^2) * d^2
		// Where r is the source radius, and d is the distance from the light. As derived here:
		//   https://imdoingitwrong.wordpress.com/2011/01/31/light-attenuation/

		float luminousFlux = GetIntensity();

		float a = sqrt(minAttenuation);
		mAttRadius = (mSourceRadius * (sqrt(luminousFlux - a))) / a;
	}
	else // Based on the basic inverse square distance formula
	{
		float luminousIntensity = GetIntensity();

		float a = minAttenuation;
		mAttRadius = sqrt(std::max(0.0f, luminousIntensity / a));
	}

	UpdateBounds();
}

void LightBase::UpdateBounds()
{
	const Transform& tfrm = GetTransform();

	switch(mType)
	{
	case LightType::Directional:
		mBounds = Sphere(tfrm.GetPosition(), std::numeric_limits<float>::infinity());
		break;
	case LightType::Radial:
		mBounds = Sphere(tfrm.GetPosition(), mAttRadius);
		break;
	case LightType::Spot:
		{
			// Note: We could use the formula for calculating the circumcircle of a triangle (after projecting the cone),
			// but the radius of the sphere is the same as in the formula we use here, yet it is much simpler with no need
			// to calculate multiple determinants. Neither are good approximations when cone angle is wide.
			Vector3 offset(0, 0, mAttRadius * 0.5f);

			// Direction along the edge of the cone, on the YZ plane (doesn't matter if we used XZ instead)
			Degree angle = Math::Clamp(mSpotAngle * 0.5f, Degree(-89), Degree(89));
			Vector3 coneDir(0, Math::Tan(angle) * mAttRadius, mAttRadius);

			// Distance between the "corner" of the cone and our center, must be the radius (provided the center is at
			// the middle of the range)
			float radius = (offset - coneDir).Length();

			Vector3 center = tfrm.GetPosition() - tfrm.GetRotation().Rotate(offset);
			mBounds = Sphere(center, radius);
		}
		break;
	default:
		break;
	}
}

void LightBase::SetTransform(const Transform& transform)
{
	if(mMobility != ObjectMobility::Movable)
		return;

	SceneActor::SetTransform(transform);
	UpdateBounds();
}

namespace bs
{
	B3D_SYNC_BLOCK_BEGIN(Light, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(mType)
		B3D_SYNC_BLOCK_ENTRY(mCastsShadows)
		B3D_SYNC_BLOCK_ENTRY(mColor)
		B3D_SYNC_BLOCK_ENTRY(mAttRadius)
		B3D_SYNC_BLOCK_ENTRY(mSourceRadius)
		B3D_SYNC_BLOCK_ENTRY(mIntensity)
		B3D_SYNC_BLOCK_ENTRY(mSpotAngle)
		B3D_SYNC_BLOCK_ENTRY(mSpotFalloffAngle)
		B3D_SYNC_BLOCK_ENTRY(mAutoAttenuation)
		B3D_SYNC_BLOCK_ENTRY(mBounds)
		B3D_SYNC_BLOCK_ENTRY(mShadowBias)
		B3D_SYNC_BLOCK_ENTRY_PACKET_BASE(SceneActor, SceneActorPacket)
	B3D_SYNC_BLOCK_END
}

Light::Light(LightType type, Color color, float intensity, float attRadius, float srcRadius, bool castsShadows, Degree spotAngle, Degree spotFalloffAngle)
	: LightBase(type, color, intensity, attRadius, srcRadius, castsShadows, spotAngle, spotFalloffAngle)
{
	// Calling virtual method is okay here because this is the most derived type
	UpdateBounds();
}

SPtr<Light> Light::Create(LightType type, Color color, float intensity, float attRadius, bool castsShadows, Degree spotAngle, Degree spotFalloffAngle)
{
	Light* handler = new(B3DAllocate<Light>())
		Light(type, color, intensity, attRadius, 0.0f, castsShadows, spotAngle, spotFalloffAngle);
	SPtr<Light> handlerPtr = B3DMakeSharedFromExisting<Light>(handler);
	handlerPtr->SetShared(handlerPtr);
	handlerPtr->Initialize();

	return handlerPtr;
}

SPtr<Light> Light::CreateEmpty()
{
	Light* handler = new(B3DAllocate<Light>()) Light();
	SPtr<Light> handlerPtr = B3DMakeSharedFromExisting<Light>(handler);
	handlerPtr->SetShared(handlerPtr);

	return handlerPtr;
}

SPtr<ct::RenderProxy> Light::CreateRenderProxy() const
{
	ct::Light* handler = new(B3DAllocate<ct::Light>())
		ct::Light(mType, mColor, mIntensity, mAttRadius, mSourceRadius, mCastsShadows, mSpotAngle, mSpotFalloffAngle);
	SPtr<ct::Light> handlerPtr = B3DMakeSharedFromExisting<ct::Light>(handler);
	handlerPtr->SetShared(handlerPtr);

	return handlerPtr;
}

RenderProxySyncPacket* Light::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	SyncPacket* const syncPacket = allocator.Construct<SyncPacket>(*this, allocator, flags);
	if(B3D_ENSURE(syncPacket))
		syncPacket->SceneActorPacket = CreateSceneActorRenderProxySyncPacket(allocator, flags);

	return syncPacket;
}

void Light::MarkRenderProxyDataDirtyInternal(ActorDirtyFlag flag)
{
	MarkRenderProxyDataDirty((u32)flag);
}

RTTITypeBase* Light::GetRttiStatic()
{
	return LightRTTI::Instance();
}

RTTITypeBase* Light::GetRtti() const
{
	return Light::GetRttiStatic();
}

namespace bs { namespace ct
{
const u32 Light::kLightConeNumSides = 20;
const u32 Light::kLightConeNumSlices = 10;

Light::Light(LightType type, Color color, float intensity, float attRadius, float srcRadius, bool castsShadows, Degree spotAngle, Degree spotFalloffAngle)
	: LightBase(type, color, intensity, attRadius, srcRadius, castsShadows, spotAngle, spotFalloffAngle), mRendererId(0)
{
}

Light::~Light()
{
	GetRenderer()->NotifyLightRemoved(this);
}

void Light::Initialize()
{
	UpdateBounds();
	GetRenderer()->NotifyLightAdded(this);

	RenderProxy::Initialize();
}

void Light::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<bs::Light::SyncPacket>();
	if(!syncPacket)
		return;

	bool oldIsActive = mActive;
	LightType oldType = mType;

	syncPacket->ApplySyncData(this);

	UpdateBounds();

	const u32 flags = syncPacket->Flags;
	if((flags & ((u32)ActorDirtyFlag::Everything | (u32)ActorDirtyFlag::Active)) != 0)
	{
		if(oldIsActive != mActive)
		{
			if(mActive)
				GetRenderer()->NotifyLightAdded(this);
			else
			{
				LightType newType = mType;
				mType = oldType;
				GetRenderer()->NotifyLightRemoved(this);
				mType = newType;
			}
		}
		else
		{
			LightType newType = mType;
			mType = oldType;
			GetRenderer()->NotifyLightRemoved(this);
			mType = newType;

			GetRenderer()->NotifyLightAdded(this);
		}
	}
	else if((flags & (u32)ActorDirtyFlag::Mobility) != 0)
	{
		GetRenderer()->NotifyLightRemoved(this);
		GetRenderer()->NotifyLightAdded(this);
	}
	else if((flags & (u32)ActorDirtyFlag::Transform) != 0)
	{
		if(mActive)
			GetRenderer()->NotifyLightUpdated(this);
	}
}
}}
