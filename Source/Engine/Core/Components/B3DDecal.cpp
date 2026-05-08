//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/B3DDecal.h"

#include "CoreObject/B3DCoreObjectSync.h"
#include "ECS/B3DRegistry.h"
#include "RTTI/B3DDecalRTTI.h"
#include "Renderer/B3DRendererScene.h"
#include "Material/B3DMaterial.h"
#include "Scene/B3DSceneInstance.h"
#include "Scene/B3DSceneObjectFragments.h"

using namespace b3d;

Bounds b3d::ComputeDecalBounds(const Vector2& size, float maxDistance, const Transform& transform)
{
	const Vector2& extents = size * 0.5f;

	AABox localAABB(
		Vector3(-extents.X, -extents.Y, -maxDistance),
		Vector3(extents.X, extents.Y, 0.0f));

	localAABB.TransformAffine(transform.GetMatrix());

	return Bounds(localAABB);
}

ecs::Decal& ecs::CreateDecal(ecs::Registry& registry, ecs::Entity entity, const TShared<RendererScene>& rendererScene, const Transform& transform)
{
	ecs::DecalECSUtility::CreateFragmentsIfMissing(registry, entity);
	registry.AddComponent<ecs::WorldTransform>(entity, ecs::WorldTransform(transform));
	rendererScene->AllocateDecalId(registry, entity);
	ecs::DecalECSUtility::MarkDirty(registry, entity);

	return registry.GetComponents<ecs::Decal>(entity);
}

void ecs::DestroyDecal(ecs::Registry& registry, ecs::Entity entity)
{
	ecs::DecalECSUtility::RemoveFragments(registry, entity);
}

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN_CUSTOM(ecs::Decal, FullSyncPacketECS, TDecalData<true>)
		B3D_SYNC_BLOCK_ENTRY(Size)
		B3D_SYNC_BLOCK_ENTRY(MaxDistance)
		B3D_SYNC_BLOCK_ENTRY(Material)
		B3D_SYNC_BLOCK_ENTRY(Layer)
		B3D_SYNC_BLOCK_ENTRY(LayerMask)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(Transform, TransformData)
	B3D_SYNC_BLOCK_END

	B3D_SYNC_BLOCK_BEGIN_CUSTOM(ecs::Decal, TransformSyncPacketECS, TDecalData<true>)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(Transform, TransformData)
	B3D_SYNC_BLOCK_END

	struct DecalFullUpdateChannel : TRendererObjectECSSyncChannel
	<
		DecalFullUpdateChannel,
		ecs::Decal::FullSyncPacketECS,
		ecs::DecalDirty,
		ecs::Decal, ecs::WorldTransform, ecs::DecalId
	>
	{
		void Write(DecalObjectStorageBase& storage, FrameAllocator& allocator)
		{
			Vector<PackedRendererId, StdFrameAlloc<PackedRendererId>> renderStatesToCreate(&allocator);
			Vector<PackedRendererId, StdFrameAlloc<PackedRendererId>> renderStatesToDestroy(&allocator);

			WritePackets(storage, allocator, [&renderStatesToCreate, &renderStatesToDestroy, &storage](ecs::Decal::FullSyncPacketECS& packet, PackedRendererId rendererId)
			{
				render::DecalProxy& proxy = storage.GetDecalProxy(rendererId);

				bool wasRegistered = proxy.mRendererId != kInvalidPackedRendererId;
				proxy.mRendererId = rendererId;
				packet.ApplySyncData(&proxy.mData);

				proxy.mTransform = packet.TransformData;
				proxy.mWorldTransformMatrix = proxy.mTransform.GetMatrix();
				proxy.mWorldTransformMatrixWithoutScale = Matrix4::TRS(proxy.mTransform.GetPosition(), proxy.mTransform.GetRotation(), Vector3::kOne);
				proxy.mBounds = ComputeDecalBounds(proxy.mData.Size, proxy.mData.MaxDistance, proxy.mTransform);

				if(wasRegistered)
					renderStatesToDestroy.push_back(rendererId);

				renderStatesToCreate.push_back(rendererId);
			});

			if(!renderStatesToDestroy.empty())
				storage.DestroyRenderState(renderStatesToDestroy);

			if(!renderStatesToCreate.empty())
				storage.CreateRenderState(renderStatesToCreate);
		}

		void CreateAndPopulatePacket(ecs::Decal& fragment, ecs::WorldTransform& transform, ecs::DecalId& id, FrameAllocator& allocator)
		{
			auto& packet = CreatePacket(id.Id, fragment, allocator, 0);
			packet.TransformData = transform;
		}
	};

	struct DecalTransformUpdateChannel : TRendererObjectECSSyncChannel
	<
		DecalTransformUpdateChannel,
		ecs::Decal::TransformSyncPacketECS,
		ecs::DecalTransformDirty,
		ecs::Decal, ecs::WorldTransform, ecs::DecalId
	>
	{
		void Write(DecalObjectStorageBase& storage, FrameAllocator& allocator)
		{
			Vector<PackedRendererId, StdFrameAlloc<PackedRendererId>> renderStatesToUpdate(&allocator);

			WritePackets(storage, allocator, [&renderStatesToUpdate, &storage](ecs::Decal::TransformSyncPacketECS& packet, PackedRendererId rendererId)
			{
				render::DecalProxy& proxy = storage.GetDecalProxy(rendererId);
				proxy.mTransform = packet.TransformData;
				proxy.mWorldTransformMatrix = proxy.mTransform.GetMatrix();
				proxy.mWorldTransformMatrixWithoutScale = Matrix4::TRS(proxy.mTransform.GetPosition(), proxy.mTransform.GetRotation(), Vector3::kOne);
				proxy.mBounds = ComputeDecalBounds(proxy.mData.Size, proxy.mData.MaxDistance, proxy.mTransform);

				renderStatesToUpdate.push_back(rendererId);
			});

			if(!renderStatesToUpdate.empty())
				storage.UpdateRenderState(renderStatesToUpdate);
		}

		void CreateAndPopulatePacket(ecs::Decal& fragment, ecs::WorldTransform& transform, ecs::DecalId& id, FrameAllocator& allocator)
		{
			auto& packet = CreatePacket(id.Id, fragment, allocator, 0);
			packet.TransformData = transform;
		}
	};

	using DecalSyncBatch = TRendererObjectECSSyncBatch<DecalFullUpdateChannel, DecalTransformUpdateChannel>;
}

// ecs::Decal fragment access

ecs::Decal& Decal::GetFragment()
{
	return GetECSRegistry()->GetComponents<ecs::Decal>(GetECSEntity());
}

const ecs::Decal& Decal::GetFragment() const
{
	return GetECSRegistry()->GetComponents<ecs::Decal>(GetECSEntity());
}

const TDecalData<false>& Decal::GetDecalData() const
{
	return GetFragment();
}

// b3d::Decal setters

void Decal::SetSize(const Vector2& size)
{
	GetFragment().Size = Vector2::Max(Vector2::kZero, size);
	MarkRenderProxyDataDirty();
	UpdateBounds();
}

void Decal::SetMaterial(const HMaterial& material)
{
	GetFragment().Material = material;
	MarkRenderProxyDataDirty();
}

void Decal::SetMaxDistance(float distance)
{
	GetFragment().MaxDistance = Math::Max(0.0f, distance);
	MarkRenderProxyDataDirty();
	UpdateBounds();
}

void Decal::SetLayerMask(u32 mask)
{
	GetFragment().LayerMask = mask;
	MarkRenderProxyDataDirty();
}

void Decal::SetLayer(u64 layer)
{
	const bool isPowerOfTwo = layer && !((layer - 1) & layer);

	if(!isPowerOfTwo)
	{
		B3D_LOG(Warning, LogRenderer, "Invalid layer provided. Only one layer bit may be set. Ignoring.");
		return;
	}

	GetFragment().Layer = layer;
	MarkRenderProxyDataDirty();
}

void Decal::UpdateBounds()
{
	const ecs::Decal& fragment = GetFragment();
	mBounds = ComputeDecalBounds(fragment.Size, fragment.MaxDistance, SceneObject()->GetTransform());
}

void Decal::MarkRenderProxyDataDirty(ComponentDirtyFlag flag)
{
	if(!SceneObject().IsValid())
		return;

	ecs::DecalECSUtility::MarkDirty(*GetECSRegistry(), GetECSEntity(), flag);
}

// b3d::Decal lifecycle

Decal::Decal(const HSceneObject& parent)
	: Component(parent)
{
	SetFlag(ComponentFlag::AlwaysRun, true);
	SetName("Decal");
	mNotifyFlags = TCF_Transform;
}

Decal::Decal()
	: Decal(nullptr)
{ }

void Decal::Initialize()
{
	SetShared(B3DStaticGameObjectCast<Decal>(mThisHandle).GetShared());
	ecs::DecalECSUtility::CreateFragmentsIfMissing(*GetECSRegistry(), GetECSEntity());

	Component::Initialize();
	CoreObject::Initialize();

	UpdateBounds();
}

void Decal::OnEnabled()
{
	const TShared<RendererScene>& rendererScene = SceneObject()->GetScene()->GetRendererScene();
	ecs::DecalECSUtility::RegisterWithRenderer(*GetECSRegistry(), GetECSEntity(), rendererScene);
}

void Decal::OnDisabled()
{
	const TShared<RendererScene>& rendererScene = SceneObject()->GetScene()->GetRendererScene();
	ecs::DecalECSUtility::UnregisterFromRenderer(*GetECSRegistry(), GetECSEntity(), rendererScene);
}

void Decal::OnDestroyed()
{
	ecs::DecalECSUtility::RemoveFragments(*GetECSRegistry(), GetECSEntity());
	CoreObject::Destroy();
}

void Decal::OnSceneChanged(SceneInstance* oldScene, ecs::Entity oldEntity)
{
	ecs::DecalECSUtility::ChangeScene(oldScene, oldEntity, *SceneObject()->GetScene(), GetECSEntity(), GetEnabled());
}

void Decal::OnTransformChanged(TransformChangedFlags flags)
{
	UpdateBounds();
	MarkRenderProxyDataDirty(ComponentDirtyFlag::Transform);
}

void Decal::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	const auto& material = GetFragment().Material;
	if(material.IsLoaded())
		dependencies.push_back(material.Get());
}

RTTIType* Decal::GetRttiStatic()
{
	return DecalRTTI::Instance();
}

RTTIType* Decal::GetRtti() const
{
	return Decal::GetRttiStatic();
}

namespace b3d::render
{
	Vector2 DecalProxy::GetWorldSize() const
	{
		const Vector3& scale = mTransform.GetScale();
		return Vector2(mData.Size.X * scale.X, mData.Size.Y * scale.Y);
	}

	float DecalProxy::GetWorldMaxDistance() const
	{
		return mData.MaxDistance * mTransform.GetScale().Z;
	}
}

namespace b3d::ecs
{
	RTTIType* Decal::GetRttiStatic() { return ECSDecalRTTI::Instance(); }
	RTTIType* Decal::GetRtti() const { return Decal::GetRttiStatic(); }
}

// DecalObjectStorageBase

void* DecalObjectStorageBase::SyncRead(ecs::Registry& registry, FrameAllocator& allocator)
{
	return DecalSyncBatch::Read(*this, registry, allocator);
}

void DecalObjectStorageBase::SyncWrite(void* rawData, FrameAllocator& allocator)
{
	DecalSyncBatch::Write(*this, rawData, allocator);
}
