//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsLight.h"

#include "BsRendererScene.h"
#include "Private/RTTI/BsLightRTTI.h"
#include "Renderer/BsRenderer.h"
#include "Scene/BsSceneObject.h"
#include "Mesh/BsMesh.h"
#include "CoreObject/BsCoreObjectSync.h"
#include "Scene/BsSceneManager.h"

using namespace b3d;

template<bool IsRenderProxy>
TLight<IsRenderProxy>::TLight()
	: mType(LightType::Radial), mCastsShadows(false), mColor(Color::kWhite), mAttRadius(10.0f), mSourceRadius(0.0f), mIntensity(100.0f), mSpotAngle(45), mSpotFalloffAngle(35.0f), mAutoAttenuation(false), mShadowBias(0.5f)
{
	UpdateAttenuationRange();
}

template<bool IsRenderProxy>
TLight<IsRenderProxy>::TLight(const SPtr<SceneInstanceType>& scene, LightType type, Color color, float intensity, float attRadius, float srcRadius, bool castsShadows, Degree spotAngle, Degree spotFalloffAngle)
	: Super(scene), mType(type), mCastsShadows(castsShadows), mColor(color), mAttRadius(attRadius), mSourceRadius(srcRadius), mIntensity(intensity), mSpotAngle(spotAngle), mSpotFalloffAngle(spotFalloffAngle), mAutoAttenuation(false), mShadowBias(0.5f)
{
	UpdateAttenuationRange();
}

template<bool IsRenderProxy>
void TLight<IsRenderProxy>::SetUseAutoAttenuation(bool enabled)
{
	mAutoAttenuation = enabled;

	if(enabled)
		UpdateAttenuationRange();

	this->MarkSceneActorRenderProxyDataDirty();
}

template<bool IsRenderProxy>
void TLight<IsRenderProxy>::SetAttenuationRadius(float radius)
{
	if(mAutoAttenuation)
		return;

	mAttRadius = radius;
	this->MarkSceneActorRenderProxyDataDirty();
	UpdateBounds();
}

template<bool IsRenderProxy>
void TLight<IsRenderProxy>::SetSourceRadius(float radius)
{
	mSourceRadius = radius;

	if(mAutoAttenuation)
		UpdateAttenuationRange();

	this->MarkSceneActorRenderProxyDataDirty();
}

template<bool IsRenderProxy>
void TLight<IsRenderProxy>::SetIntensity(float intensity)
{
	mIntensity = intensity;

	if(mAutoAttenuation)
		UpdateAttenuationRange();

	this->MarkSceneActorRenderProxyDataDirty();
}

template<bool IsRenderProxy>
float TLight<IsRenderProxy>::GetLuminance() const
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

template<bool IsRenderProxy>
void TLight<IsRenderProxy>::UpdateAttenuationRange()
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

template<bool IsRenderProxy>
void TLight<IsRenderProxy>::UpdateBounds()
{
	const Transform& tfrm = this->GetTransform();

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

template<bool IsRenderProxy>
void TLight<IsRenderProxy>::SetTransform(const Transform& transform)
{
	if(this->mMobility != ObjectMobility::Movable)
		return;

	SceneActor::SetTransform(transform);
	UpdateBounds();
}

template class TLight<true>;
template class TLight<false>;

namespace b3d
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

Light::Light(const SPtr<SceneInstance>& scene, LightType type, Color color, float intensity, float attRadius, float srcRadius, bool castsShadows, Degree spotAngle, Degree spotFalloffAngle)
	: TLight(scene, type, color, intensity, attRadius, srcRadius, castsShadows, spotAngle, spotFalloffAngle)
{
	// Calling virtual method is okay here because this is the most derived type
	UpdateBounds();
}

SPtr<Light> Light::Create(const SPtr<SceneInstance>& scene, LightType type, Color color, float intensity, float attRadius, bool castsShadows, Degree spotAngle, Degree spotFalloffAngle)
{
	Light* light = new(B3DAllocate<Light>())
		Light(scene, type, color, intensity, attRadius, 0.0f, castsShadows, spotAngle, spotFalloffAngle);
	SPtr<Light> lightShared = B3DMakeSharedFromExisting<Light>(light);
	lightShared->SetShared(lightShared);
	lightShared->Initialize();

	return lightShared;
}

SPtr<Light> Light::CreateEmpty()
{
	Light* light = new(B3DAllocate<Light>()) Light();
	SPtr<Light> lightShared = B3DMakeSharedFromExisting<Light>(light);
	lightShared->SetShared(lightShared);

	return lightShared;
}

SPtr<render::RenderProxy> Light::CreateRenderProxy() const
{
	render::Light* renderProxy = new(B3DAllocate<render::Light>())
		render::Light(B3DGetRenderProxy(mSceneInstance), mType, mColor, mIntensity, mAttRadius, mSourceRadius, mCastsShadows, mSpotAngle, mSpotFalloffAngle);
	SPtr<render::Light> renderProxyShared = B3DMakeSharedFromExisting<render::Light>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	return renderProxyShared;
}

RenderProxySyncPacket* Light::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	SyncPacket* const syncPacket = allocator.Construct<SyncPacket>(*this, allocator, flags);
	if(B3D_ENSURE(syncPacket))
		syncPacket->SceneActorPacket = CreateSceneActorRenderProxySyncPacket(allocator, flags);

	return syncPacket;
}

void Light::MarkSceneActorRenderProxyDataDirty(ActorDirtyFlag flag)
{
	MarkRenderProxyDataDirty((u32)flag);
}

RTTIType* Light::GetRttiStatic()
{
	return LightRTTI::Instance();
}

RTTIType* Light::GetRtti() const
{
	return Light::GetRttiStatic();
}

namespace b3d { namespace render
{
const u32 Light::kLightConeNumSides = 20;
const u32 Light::kLightConeNumSlices = 10;

Light::Light(const SPtr<SceneInstance>& scene, LightType type, Color color, float intensity, float attRadius, float srcRadius, bool castsShadows, Degree spotAngle, Degree spotFalloffAngle)
	: TLight(scene, type, color, intensity, attRadius, srcRadius, castsShadows, spotAngle, spotFalloffAngle), mRendererId(0)
{
}

Light::~Light()
{
	const SPtr<RendererScene>& rendererScene = mSceneInstance->GetRendererScene();
	rendererScene->UnregisterLight(this);
}

void Light::Initialize()
{
	UpdateBounds();

	const SPtr<RendererScene>& rendererScene = mSceneInstance->GetRendererScene();
	rendererScene->RegisterLight(this);

	RenderProxy::Initialize();
}

void Light::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<b3d::Light::SyncPacket>();
	if(!syncPacket)
		return;

	bool oldIsActive = mActive;
	LightType oldType = mType;

	syncPacket->ApplySyncData(this);

	UpdateBounds();

	const SPtr<RendererScene>& rendererScene = mSceneInstance->GetRendererScene();

	const u32 flags = syncPacket->Flags;
	if((flags & ((u32)ActorDirtyFlag::Everything | (u32)ActorDirtyFlag::Active)) != 0)
	{
		if(oldIsActive != mActive)
		{
			if(mActive)
				rendererScene->RegisterLight(this);
			else
			{
				LightType newType = mType;
				mType = oldType;
				rendererScene->UnregisterLight(this);
				mType = newType;
			}
		}
		else
		{
			LightType newType = mType;
			mType = oldType;
			rendererScene->UnregisterLight(this);
			mType = newType;

			rendererScene->RegisterLight(this);
		}
	}
	else if((flags & (u32)ActorDirtyFlag::Mobility) != 0)
	{
		rendererScene->UnregisterLight(this);
		rendererScene->RegisterLight(this);
	}
	else if((flags & (u32)ActorDirtyFlag::Transform) != 0)
	{
		if(mActive)
			rendererScene->UpdateLight(this);
	}
}
}}
