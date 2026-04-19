//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/B3DLight.h"

#include "CoreObject/B3DCoreObjectSync.h"
#include "ECS/B3DRegistry.h"
#include "Image/B3DColor.h"
#include "RTTI/B3DLightRTTI.h"
#include "Renderer/B3DRendererScene.h"
#include "Scene/B3DSceneInstance.h"
#include "Scene/B3DSceneObjectFragments.h"

using namespace b3d;

ecs::Light& ecs::CreateLight(ecs::Registry& registry, ecs::Entity entity, const SPtr<RendererScene>& rendererScene, const Transform& transform)
{
	ecs::LightECSUtility::CreateFragmentsIfMissing(registry, entity);
	registry.AddComponent<ecs::WorldTransform>(entity, ecs::WorldTransform(transform));
	rendererScene->AllocateLightId(registry, entity);
	ecs::LightECSUtility::MarkDirty(registry, entity);

	return registry.GetComponents<ecs::Light>(entity);
}

void ecs::DestroyLight(ecs::Registry& registry, ecs::Entity entity)
{
	ecs::LightECSUtility::RemoveFragments(registry, entity);
}

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN_CUSTOM(ecs::Light, FullSyncPacket, TLightData<true>)
		B3D_SYNC_BLOCK_ENTRY(Type)
		B3D_SYNC_BLOCK_ENTRY(CastsShadows)
		B3D_SYNC_BLOCK_ENTRY(LightColor)
		B3D_SYNC_BLOCK_ENTRY(AttRadius)
		B3D_SYNC_BLOCK_ENTRY(SourceRadius)
		B3D_SYNC_BLOCK_ENTRY(Intensity)
		B3D_SYNC_BLOCK_ENTRY(SpotAngle)
		B3D_SYNC_BLOCK_ENTRY(SpotFalloffAngle)
		B3D_SYNC_BLOCK_ENTRY(AutoAttenuation)
		B3D_SYNC_BLOCK_ENTRY(Bounds)
		B3D_SYNC_BLOCK_ENTRY(ShadowBias)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(Transform, TransformData)
	B3D_SYNC_BLOCK_END

	B3D_SYNC_BLOCK_BEGIN_CUSTOM(ecs::Light, TransformSyncPacket, TLightData<true>)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(Transform, TransformData)
	B3D_SYNC_BLOCK_END

	struct LightFullUpdateChannel : TRendererObjectECSSyncChannel
	<
		LightFullUpdateChannel,
		ecs::Light::FullSyncPacket,
		ecs::LightDirty,
		ecs::Light, ecs::WorldTransform, ecs::LightId
	>
	{
		void Write(LightObjectStorageBase& storage, FrameAllocator& allocator)
		{
			Vector<PackedRendererId, StdFrameAlloc<PackedRendererId>> renderStatesToCreate(&allocator);
			Vector<PackedRendererId, StdFrameAlloc<PackedRendererId>> renderStatesToDestroy(&allocator);

			WritePackets(storage, allocator, [&renderStatesToCreate, &renderStatesToDestroy, &storage](ecs::Light::FullSyncPacket& packet, PackedRendererId rendererId)
			{
				render::LightProxy& proxy = storage.GetLightProxy(rendererId);

				bool wasRegistered = proxy.mRendererId != kInvalidPackedRendererId;
				proxy.mRendererId = rendererId;
				packet.ApplySyncData(&proxy.mData);

				proxy.mTransform = packet.TransformData;
				proxy.mData.ComputeBounds(proxy.mTransform);
				proxy.mBounds = proxy.mData.Bounds;

				if(wasRegistered)
					renderStatesToDestroy.push_back(rendererId);

				renderStatesToCreate.push_back(rendererId);
			});

			if(!renderStatesToDestroy.empty())
				storage.DestroyRenderState(renderStatesToDestroy);

			if(!renderStatesToCreate.empty())
				storage.CreateRenderState(renderStatesToCreate);
		}

		void CreateAndPopulatePacket(ecs::Light& fragment, ecs::WorldTransform& transform, ecs::LightId& id, FrameAllocator& allocator)
		{
			auto& packet = CreatePacket(id.Id, fragment, allocator, 0);
			packet.TransformData = transform;
		}
	};

	struct LightTransformUpdateChannel : TRendererObjectECSSyncChannel
	<
		LightTransformUpdateChannel,
		ecs::Light::TransformSyncPacket,
		ecs::LightTransformDirty,
		ecs::Light, ecs::WorldTransform, ecs::LightId
	>
	{
		void Write(LightObjectStorageBase& storage, FrameAllocator& allocator)
		{
			Vector<PackedRendererId, StdFrameAlloc<PackedRendererId>> renderStatesToUpdate(&allocator);

			WritePackets(storage, allocator, [&renderStatesToUpdate, &storage](ecs::Light::TransformSyncPacket& packet, PackedRendererId rendererId)
			{
				render::LightProxy& proxy = storage.GetLightProxy(rendererId);
				proxy.mTransform = packet.TransformData;
				proxy.mData.ComputeBounds(proxy.mTransform);
				proxy.mBounds = proxy.mData.Bounds;

				renderStatesToUpdate.push_back(rendererId);
			});

			if(!renderStatesToUpdate.empty())
				storage.UpdateRenderState(renderStatesToUpdate);
		}

		void CreateAndPopulatePacket(ecs::Light& fragment, ecs::WorldTransform& transform, ecs::LightId& id, FrameAllocator& allocator)
		{
			auto& packet = CreatePacket(id.Id, fragment, allocator, 0);
			packet.TransformData = transform;
		}
	};

	using LightSyncBatch = TRendererObjectECSSyncBatch<LightFullUpdateChannel, LightTransformUpdateChannel>;
}

template <bool IsRenderProxy>
void TLightData<IsRenderProxy>::ComputeBounds(const Transform& transform)
{
	switch(Type)
	{
	case LightType::Directional:
		Bounds = Sphere(transform.GetPosition(), std::numeric_limits<float>::infinity());
		break;
	case LightType::Radial:
		Bounds = Sphere(transform.GetPosition(), AttRadius);
		break;
	case LightType::Spot:
		{
			// Note: We could use the formula for calculating the circumcircle of a triangle (after projecting the cone),
			// but the radius of the sphere is the same as in the formula we use here, yet it is much simpler with no need
			// to calculate multiple determinants. Neither are good approximations when cone angle is wide.
			Vector3 offset(0, 0, AttRadius * 0.5f);

			// Direction along the edge of the cone, on the YZ plane (doesn't matter if we used XZ instead)
			Degree angle = Math::Clamp(SpotAngle * 0.5f, Degree(-89), Degree(89));
			Vector3 coneDir(0, Math::Tan(angle) * AttRadius, AttRadius);

			// Distance between the "corner" of the cone and our center, must be the radius (provided the center is at
			// the middle of the range)
			float radius = (offset - coneDir).Length();

			Vector3 center = transform.GetPosition() - transform.GetRotation().Rotate(offset);
			Bounds = Sphere(center, radius);
		}
		break;
	default:
		break;
	}
}

template void TLightData<true>::ComputeBounds(const Transform& transform);
template void TLightData<false>::ComputeBounds(const Transform& transform);

template <bool IsRenderProxy>
float TLightData<IsRenderProxy>::ComputeLuminance() const
{
	float radiusSquared = SourceRadius * SourceRadius;

	switch(Type)
	{
	case LightType::Radial:
		if(SourceRadius > 0.0f)
			return Intensity / (4 * radiusSquared * Math::kPi); // Luminous flux -> luminance
		else
			return Intensity / (4 * Math::kPi); // Luminous flux -> luminous intensity
	case LightType::Spot:
		{
			if(SourceRadius > 0.0f)
				return Intensity / (radiusSquared * Math::kPi); // Luminous flux -> luminance
			else
			{
				// Note: Consider using the simpler conversion I / PI to match with the area-light conversion
				float cosTotalAngle = Math::Cos(SpotAngle);
				float cosFalloffAngle = Math::Cos(SpotFalloffAngle);

				// Luminous flux -> luminous intensity
				return Intensity / (Math::kTwoPi * (1.0f - (cosFalloffAngle + cosTotalAngle) * 0.5f));
			}
		}
	case LightType::Directional:
		if(SourceRadius > 0.0f)
		{
			// Use cone solid angle formulae to calculate disc solid angle
			float solidAngle = Math::kTwoPi * (1 - cos(SourceRadius * Math::kDeG2Rad));
			return Intensity / solidAngle; // Illuminance -> luminance
		}
		else
			return Intensity; // In luminance units by default
	default:
		return 0.0f;
	}
}

template float TLightData<true>::ComputeLuminance() const;
template float TLightData<false>::ComputeLuminance() const;

template <bool IsRenderProxy>
void TLightData<IsRenderProxy>::ComputeAttenuationRange(const Transform& transform)
{
	// Value to which intensity needs to drop in order for the light contribution to fade out to zero
	const float minAttenuation = 0.2f;

	if(SourceRadius > 0.0f)
	{
		// Inverse of the attenuation formula for area lights:
		//   a = I / (1 + (2/r) * d + (1/r^2) * d^2
		// Where r is the source radius, and d is the distance from the light. As derived here:
		//   https://imdoingitwrong.wordpress.com/2011/01/31/light-attenuation/

		float luminousFlux = Intensity;

		float a = sqrt(minAttenuation);
		AttRadius = (SourceRadius * (sqrt(luminousFlux - a))) / a;
	}
	else // Based on the basic inverse square distance formula
	{
		float luminousIntensity = Intensity;

		float a = minAttenuation;
		AttRadius = sqrt(std::max(0.0f, luminousIntensity / a));
	}

	ComputeBounds(transform);
}

template void TLightData<true>::ComputeAttenuationRange(const Transform& transform);
template void TLightData<false>::ComputeAttenuationRange(const Transform& transform);

// ecs::Light fragment access

ecs::Light& Light::GetFragment()
{
	return GetECSRegistry()->GetComponents<ecs::Light>(GetECSEntity());
}

const ecs::Light& Light::GetFragment() const
{
	return GetECSRegistry()->GetComponents<ecs::Light>(GetECSEntity());
}

const TLightData<false>& Light::GetLightData() const
{
	return GetFragment();
}

// b3d::Light setters

void Light::SetType(LightType type)
{
	GetFragment().Type = type;
	MarkRenderProxyDataDirty();
	UpdateBounds();
}

void Light::SetCastsShadow(bool castsShadow)
{
	GetFragment().CastsShadows = castsShadow;
	MarkRenderProxyDataDirty();
}

void Light::SetShadowBias(float bias)
{
	GetFragment().ShadowBias = bias;
	MarkRenderProxyDataDirty();
}

void Light::SetColor(const Color& color)
{
	GetFragment().LightColor = color;
	MarkRenderProxyDataDirty();
}

void Light::SetAttenuationRadius(float radius)
{
	ecs::Light& fragment = GetFragment();
	if(fragment.AutoAttenuation)
		return;

	fragment.AttRadius = radius;
	MarkRenderProxyDataDirty();
	UpdateBounds();
}

void Light::SetSourceRadius(float radius)
{
	ecs::Light& fragment = GetFragment();
	fragment.SourceRadius = radius;

	if(fragment.AutoAttenuation)
		UpdateAttenuationRange();

	MarkRenderProxyDataDirty();
}

void Light::SetUseAutoAttenuation(bool enabled)
{
	ecs::Light& fragment = GetFragment();
	fragment.AutoAttenuation = enabled;

	if(enabled)
		UpdateAttenuationRange();

	MarkRenderProxyDataDirty();
}

void Light::SetIntensity(float intensity)
{
	ecs::Light& fragment = GetFragment();
	fragment.Intensity = intensity;

	if(fragment.AutoAttenuation)
		UpdateAttenuationRange();

	MarkRenderProxyDataDirty();
}

void Light::SetSpotAngle(const Degree& spotAngle)
{
	GetFragment().SpotAngle = spotAngle;
	MarkRenderProxyDataDirty();
	UpdateBounds();
}

void Light::SetSpotFalloffAngle(const Degree& spotFallofAngle)
{
	GetFragment().SpotFalloffAngle = spotFallofAngle;
	MarkRenderProxyDataDirty();
	UpdateBounds();
}

float Light::GetLuminance() const
{
	return GetFragment().ComputeLuminance();
}

void Light::UpdateBounds()
{
	GetFragment().ComputeBounds(SceneObject()->GetTransform());
}

void Light::UpdateAttenuationRange()
{
	GetFragment().ComputeAttenuationRange(SceneObject()->GetTransform());
}

void Light::MarkRenderProxyDataDirty(ComponentDirtyFlag flag)
{
	if(!SceneObject().IsValid())
		return;

	ecs::LightECSUtility::MarkDirty(*GetECSRegistry(), GetECSEntity(), flag);
}

// b3d::Light lifecycle

Light::Light(const HSceneObject& parent)
	: Component(parent)
{
	SetFlag(ComponentFlag::AlwaysRun, true);
	SetName("Light");
}

Light::Light()
	: Light(nullptr)
{ }

void Light::Initialize()
{
	SetShared(B3DStaticGameObjectCast<Light>(mThisHandle).GetShared());
	ecs::LightECSUtility::CreateFragmentsIfMissing(*GetECSRegistry(), GetECSEntity());
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
	const SPtr<RendererScene>& rendererScene = SceneObject()->GetScene()->GetRendererScene();
	ecs::LightECSUtility::RegisterWithRenderer(*GetECSRegistry(), GetECSEntity(), rendererScene);
}

void Light::OnDisabled()
{
	const SPtr<RendererScene>& rendererScene = SceneObject()->GetScene()->GetRendererScene();
	ecs::LightECSUtility::UnregisterFromRenderer(*GetECSRegistry(), GetECSEntity(), rendererScene);
}

void Light::OnDestroyed()
{
	ecs::LightECSUtility::RemoveFragments(*GetECSRegistry(), GetECSEntity());
	CoreObject::Destroy();
}

void Light::OnSceneChanged(SceneInstance* oldScene, ecs::Entity oldEntity)
{
	ecs::LightECSUtility::ChangeScene(
		oldScene, oldEntity, *SceneObject()->GetScene(), GetECSEntity(), GetEnabled());
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

namespace b3d::ecs
{
	RTTIType* Light::GetRttiStatic() { return ECSLightRTTI::Instance(); }
	RTTIType* Light::GetRtti() const { return Light::GetRttiStatic(); }
}

// LightObjectStorageBase

void* LightObjectStorageBase::SyncRead(ecs::Registry& registry, FrameAllocator& allocator)
{
	return LightSyncBatch::Read(*this, registry, allocator);
}

void LightObjectStorageBase::SyncWrite(void* rawData, FrameAllocator& allocator)
{
	LightSyncBatch::Write(*this, rawData, allocator);
}

