//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/B3DLight.h"

#include "CoreObject/B3DCoreObjectSync.h"
#include "Image/B3DColor.h"
#include "RTTI/B3DLightRTTI.h"
#include "Renderer/B3DRendererScene.h"
#include "Scene/B3DSceneInstance.h"

using namespace b3d;

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(Light, FullSyncPacket)
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
		B3D_SYNC_BLOCK_ENTRY_CUSTOM_SETTER(bool, mActive)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM_SETTER(SPtr<SceneInstance>, mSceneInstance)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM_SETTER(Transform, mTransform)
	B3D_SYNC_BLOCK_END

	B3D_SYNC_BLOCK_BEGIN(Light, TransformSyncPacket)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM_SETTER(Transform, mTransform)
	B3D_SYNC_BLOCK_END
}

template<bool IsRenderProxy>
TLight<IsRenderProxy>::TLight()
{ }

template<bool IsRenderProxy>
TLight<IsRenderProxy>::TLight(LightType type, Color color, float intensity, float attRadius, float srcRadius, bool castsShadows, Degree spotAngle, Degree spotFalloffAngle)
	: mType(type), mCastsShadows(castsShadows), mColor(color), mAttRadius(attRadius), mSourceRadius(srcRadius), mIntensity(intensity), mSpotAngle(spotAngle), mSpotFalloffAngle(spotFalloffAngle)
{ }

template<bool IsRenderProxy>
void TLight<IsRenderProxy>::SetUseAutoAttenuation(bool enabled)
{
	mAutoAttenuation = enabled;

	if(enabled)
		UpdateAttenuationRange();

	MarkRenderProxyDataDirty();
}

template<bool IsRenderProxy>
void TLight<IsRenderProxy>::SetAttenuationRadius(float radius)
{
	if(mAutoAttenuation)
		return;

	mAttRadius = radius;
	MarkRenderProxyDataDirty();
	UpdateBounds();
}

template<bool IsRenderProxy>
void TLight<IsRenderProxy>::SetSourceRadius(float radius)
{
	mSourceRadius = radius;

	if(mAutoAttenuation)
		UpdateAttenuationRange();

	MarkRenderProxyDataDirty();
}

template<bool IsRenderProxy>
void TLight<IsRenderProxy>::SetIntensity(float intensity)
{
	mIntensity = intensity;

	if(mAutoAttenuation)
		UpdateAttenuationRange();

	MarkRenderProxyDataDirty();
}

template<bool IsRenderProxy>
float TLight<IsRenderProxy>::GetLuminance() const
{
	float radiusSquared = mSourceRadius * mSourceRadius;

	switch(mType)
	{
	case LightType::Radial:
		if(mSourceRadius > 0.0f)
			return mIntensity / (4 * radiusSquared * Math::kPi); // Luminous flux -> luminance
		else
			return mIntensity / (4 * Math::kPi); // Luminous flux -> luminous intensity
	case LightType::Spot:
		{
			if(mSourceRadius > 0.0f)
				return mIntensity / (radiusSquared * Math::kPi); // Luminous flux -> luminance
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
	const Transform& transform = this->GetTransform();

	switch(mType)
	{
	case LightType::Directional:
		mBounds = Sphere(transform.GetPosition(), std::numeric_limits<float>::infinity());
		break;
	case LightType::Radial:
		mBounds = Sphere(transform.GetPosition(), mAttRadius);
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

			Vector3 center = transform.GetPosition() - transform.GetRotation().Rotate(offset);
			mBounds = Sphere(center, radius);
		}
		break;
	default:
		break;
	}
}

template <bool IsRenderProxy>
void TLight<IsRenderProxy>::MarkRenderProxyDataDirty(ComponentDirtyFlag flag)
{
	if constexpr(!IsRenderProxy)
		CoreObject::MarkRenderProxyDataDirty((u32)flag);
}

template <bool IsRenderProxy>
const Transform& TLight<IsRenderProxy>::GetTransform() const
{
	if constexpr(!IsRenderProxy)
		return static_cast<const Light*>(this)->SceneObject()->GetTransform();
	else
		return static_cast<const render::Light*>(this)->GetWorldTransform();
}

template class TLight<true>;
template class TLight<false>;

Light::Light(const HSceneObject& parent)
	: Component(parent)
{
	SetFlag(ComponentFlag::AlwaysRun, true);
	SetName("Light");
}

Light::Light()
	: Light(nullptr)
{ }

SPtr<render::RenderProxy> Light::CreateRenderProxy() const
{
	const SPtr<SceneInstance>& scene = SceneObject()->GetScene();

	render::Light* renderProxy = new(B3DAllocate<render::Light>()) render::Light(B3DGetRenderProxy(scene), mType, mColor, mIntensity, mAttRadius, mSourceRadius, mCastsShadows, mSpotAngle, mSpotFalloffAngle);
	SPtr<render::Light> renderProxyShared = B3DMakeSharedFromExisting<render::Light>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	return renderProxyShared;
}

RenderProxySyncPacket* Light::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	if(flags != (u32)ComponentDirtyFlag::Transform)
	{
		FullSyncPacket* const syncPacket = allocator.Construct<FullSyncPacket>(*this, allocator, flags);
		syncPacket->mActive = GetEnabled();
		syncPacket->mSceneInstance = B3DGetRenderProxy(SceneObject()->GetScene());
		syncPacket->mTransform = SceneObject()->GetTransform();

		return syncPacket;
	}
	else
	{
		TransformSyncPacket* const syncPacket = allocator.Construct<TransformSyncPacket>(*this, allocator, flags);
		syncPacket->mTransform = SceneObject()->GetTransform();

		return syncPacket;
	}
}

void Light::Initialize()
{
	SetShared(B3DStaticGameObjectCast<Light>(mThisHandle).GetShared());

	Component::Initialize();
	CoreObject::Initialize();

	UpdateAttenuationRange();
}

void Light::OnCreated()
{
	UpdateBounds();
}

void Light::OnEnabled()
{
	MarkRenderProxyDataDirty();
}

void Light::OnDisabled()
{
	MarkRenderProxyDataDirty();
}

void Light::OnDestroyed()
{
	CoreObject::Destroy();
}

void Light::OnTransformChanged(TransformChangedFlags flags)
{
	UpdateBounds();

	MarkRenderProxyDataDirty(ComponentDirtyFlag::Transform);
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
	: TLight(type, color, intensity, attRadius, srcRadius, castsShadows, spotAngle, spotFalloffAngle), mRendererId(0), mSceneInstance(scene)
{
	UpdateAttenuationRange();
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
	RenderProxySyncPacket* const syncPacket = data.GetSyncPacket();
	if(syncPacket == nullptr)
		return;

	bool previousActiveState = mActive;
	LightType previousType = mType;

	syncPacket->ApplySyncData(this);

	UpdateBounds();

	const SPtr<RendererScene>& rendererScene = mSceneInstance->GetRendererScene();

	const u32 flags = syncPacket->Flags;
	const u32 updateEverythingFlag = ~(u32)ComponentDirtyFlag::Transform;
	if((flags & updateEverythingFlag) != 0)
	{
		if(previousActiveState != mActive)
		{
			if(mActive)
				rendererScene->RegisterLight(this);
			else
			{
				LightType currentType = mType;
				mType = previousType;
				rendererScene->UnregisterLight(this);
				mType = currentType;
			}
		}
		else
		{
			LightType currentType = mType;
			mType = previousType;
			rendererScene->UnregisterLight(this);
			mType = currentType;

			rendererScene->RegisterLight(this);
		}
	}
	else
	{
		if(mActive)
			rendererScene->UpdateLight(this);
	}
}
}}
