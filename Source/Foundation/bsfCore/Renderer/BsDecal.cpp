//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsDecal.h"

#include "BsRendererScene.h"
#include "Private/RTTI/BsDecalRTTI.h"
#include "RTTI/BsMathRTTI.h"
#include "Scene/BsSceneObject.h"
#include "Renderer/BsRenderer.h"
#include "Material/BsMaterial.h"
#include "CoreObject/BsCoreObjectSync.h"
#include "Scene/BsSceneManager.h"

using namespace b3d;

template<bool IsRenderProxy>
TDecal<IsRenderProxy>::TDecal()
{
	UpdateBounds();
}

template<bool IsRenderProxy>
TDecal<IsRenderProxy>::TDecal(const SPtr<SceneInstanceType>& scene, const MaterialType& material, const Vector2& size, float maxDistance)
	: Super(scene), mMaterial(material), mSize(size), mMaxDistance(maxDistance)
{
	UpdateBounds();
}

template<bool IsRenderProxy>
void TDecal<IsRenderProxy>::SetLayer(u64 layer)
{
	const bool isPow2 = layer && !((layer - 1) & layer);

	if(!isPow2)
	{
		B3D_LOG(Warning, Renderer, "Invalid layer provided. Only one layer bit may be set. Ignoring.");
		return;
	}

	mLayer = layer;
	SceneActor::MarkSceneActorRenderProxyDataDirty();
}

template<bool IsRenderProxy>
void TDecal<IsRenderProxy>::SetTransform(const Transform& transform)
{
	if(SceneActor::mMobility != ObjectMobility::Movable)
		return;

	SceneActor::mTransform = transform;
	mTfrmMatrix = transform.GetMatrix();
	mTfrmMatrixNoScale = Matrix4::TRS(transform.GetPosition(), transform.GetRotation(), Vector3::kOne);

	SceneActor::MarkSceneActorRenderProxyDataDirty(ActorDirtyFlag::Transform);
}

template<bool IsRenderProxy>
void TDecal<IsRenderProxy>::UpdateBounds()
{
	const Vector2& extents = mSize * 0.5f;

	AABox localAABB(
		Vector3(-extents.X, -extents.Y, -mMaxDistance),
		Vector3(extents.X, extents.Y, 0.0f));

	localAABB.TransformAffine(mTfrmMatrix);

	mBounds = Bounds(localAABB, Sphere(localAABB.GetCenter(), localAABB.GetRadius()));
}

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(Decal, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(mSize)
		B3D_SYNC_BLOCK_ENTRY(mMaxDistance)
		B3D_SYNC_BLOCK_ENTRY(mMaterial)
		B3D_SYNC_BLOCK_ENTRY(mBounds)
		B3D_SYNC_BLOCK_ENTRY(mLayer)
		B3D_SYNC_BLOCK_ENTRY(mLayerMask)
		B3D_SYNC_BLOCK_ENTRY_PACKET_BASE(SceneActor, SceneActorPacket)
	B3D_SYNC_BLOCK_END
}

Decal::Decal(const SPtr<SceneInstance>& scene, const HMaterial& material, const Vector2& size, float maxDistance)
	: TDecal(scene, material, size, maxDistance)
{
	// Calling virtual method is okay here because this is the most derived type
	UpdateBounds();
}

SPtr<Decal> Decal::Create(const SPtr<SceneInstance>& scene, const HMaterial& material, const Vector2& size, float maxDistance)
{
	Decal* decal = new(B3DAllocate<Decal>()) Decal(scene, material, size, maxDistance);
	SPtr<Decal> decalPtr = B3DMakeSharedFromExisting<Decal>(decal);
	decalPtr->SetShared(decalPtr);
	decalPtr->Initialize();

	return decalPtr;
}

SPtr<Decal> Decal::CreateEmpty()
{
	Decal* decal = new(B3DAllocate<Decal>()) Decal();
	SPtr<Decal> decalPtr = B3DMakeSharedFromExisting<Decal>(decal);
	decalPtr->SetShared(decalPtr);

	return decalPtr;
}

SPtr<render::RenderProxy> Decal::CreateRenderProxy() const
{
	SPtr<render::Material> material = B3DGetRenderProxy(mMaterial);

	render::Decal* renderProxy = new(B3DAllocate<render::Decal>()) render::Decal(B3DGetRenderProxy(mSceneInstance), material, mSize, mMaxDistance);
	SPtr<render::Decal> renderProxyShared = B3DMakeSharedFromExisting<render::Decal>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	return renderProxyShared;
}

void Decal::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	if(mMaterial.IsLoaded())
		dependencies.push_back(mMaterial.Get());
}

RenderProxySyncPacket* Decal::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	SyncPacket* const syncPacket = allocator.Construct<SyncPacket>(*this, allocator, flags);
	if(B3D_ENSURE(syncPacket))
		syncPacket->SceneActorPacket = CreateSceneActorRenderProxySyncPacket(allocator, flags);

	return syncPacket;
}

void Decal::MarkSceneActorRenderProxyDataDirty(ActorDirtyFlag flags)
{
	MarkRenderProxyDataDirty((u32)flags);
}

RTTIType* Decal::GetRttiStatic()
{
	return DecalRTTI::Instance();
}

RTTIType* Decal::GetRtti() const
{
	return Decal::GetRttiStatic();
}

template class TDecal<true>;
template class TDecal<false>;

namespace b3d { namespace render
{
Decal::Decal(const SPtr<SceneInstance>& scene, const SPtr<Material>& material, const Vector2& size, float maxDistance)
	: TDecal(scene, material, size, maxDistance)
{}

Decal::~Decal()
{
	const SPtr<RendererScene>& rendererScene = mSceneInstance->GetRendererScene();
	rendererScene->UnregisterDecal(this);
}

void Decal::Initialize()
{
	UpdateBounds();

	const SPtr<RendererScene>& rendererScene = mSceneInstance->GetRendererScene();
	rendererScene->RegisterDecal(this);

	RenderProxy::Initialize();
}

void Decal::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<b3d::Decal::SyncPacket>();
	if(!syncPacket)
		return;

	bool oldIsActive = mActive;
	syncPacket->ApplySyncData(this);

	mTfrmMatrix = mTransform.GetMatrix();
	mTfrmMatrixNoScale = Matrix4::TRS(mTransform.GetPosition(), mTransform.GetRotation(), Vector3::kOne);

	UpdateBounds();

	const SPtr<RendererScene>& rendererScene = mSceneInstance->GetRendererScene();
	if(syncPacket->Flags == (u32)ActorDirtyFlag::Transform)
	{
		if(mActive)
			rendererScene->UpdateDecal(this);
	}
	else
	{
		if(oldIsActive != mActive)
		{
			if(mActive)
				rendererScene->RegisterDecal(this);
			else
				rendererScene->UnregisterDecal(this);
		}
		else
		{
			rendererScene->UnregisterDecal(this);
			rendererScene->RegisterDecal(this);
		}
	}
}
}}
