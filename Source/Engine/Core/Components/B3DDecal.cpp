//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/B3DDecal.h"

#include "CoreObject/B3DCoreObjectSync.h"
#include "RTTI/B3DDecalRTTI.h"
#include "Renderer/B3DRendererScene.h"
#include "Material/B3DMaterial.h"
#include "Scene/B3DSceneInstance.h"

using namespace b3d;

template<bool IsRenderProxy>
TDecal<IsRenderProxy>::TDecal()
{
	UpdateBounds();
}

template<bool IsRenderProxy>
void TDecal<IsRenderProxy>::SetLayer(u64 layer)
{
	const bool isPowerOfTwo = layer && !((layer - 1) & layer);

	if(!isPowerOfTwo)
	{
		B3D_LOG(Warning, LogRenderer, "Invalid layer provided. Only one layer bit may be set. Ignoring.");
		return;
	}

	mLayer = layer;
	MarkRenderProxyDataDirty();
}

template<bool IsRenderProxy>
void TDecal<IsRenderProxy>::UpdateBounds()
{
	const Vector2& extents = mSize * 0.5f;

	AABox localAABB(
		Vector3(-extents.X, -extents.Y, -mMaxDistance),
		Vector3(extents.X, extents.Y, 0.0f));

	localAABB.TransformAffine(mWorldTransformMatrix);

	mBounds = Bounds(localAABB);
}

template <bool IsRenderProxy>
void TDecal<IsRenderProxy>::MarkRenderProxyDataDirty(ComponentDirtyFlag flag)
{
	if constexpr(!IsRenderProxy)
		CoreObject::MarkRenderProxyDataDirty((u32)flag);
}

template <bool IsRenderProxy>
void TDecal<IsRenderProxy>::MarkCoreObjectDependenciesDirty()
{
	if constexpr(!IsRenderProxy)
		CoreObject::MarkDependenciesDirty();
}

template class TDecal<true>;
template class TDecal<false>;

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(Decal, FullSyncPacket)
		B3D_SYNC_BLOCK_ENTRY(mSize)
		B3D_SYNC_BLOCK_ENTRY(mMaxDistance)
		B3D_SYNC_BLOCK_ENTRY(mMaterial)
		B3D_SYNC_BLOCK_ENTRY(mBounds)
		B3D_SYNC_BLOCK_ENTRY(mLayer)
		B3D_SYNC_BLOCK_ENTRY(mLayerMask)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM_SETTER(bool, mActive)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM_SETTER(SPtr<SceneInstance>, mSceneInstance)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM_SETTER(Transform, mTransform)
	B3D_SYNC_BLOCK_END

	B3D_SYNC_BLOCK_BEGIN(Decal, TransformSyncPacket)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM_SETTER(Transform, mTransform)
	B3D_SYNC_BLOCK_END
}

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

	Component::Initialize();
	CoreObject::Initialize();
}

void Decal::OnEnabled()
{
	MarkRenderProxyDataDirty();
}

void Decal::OnDisabled()
{
	MarkRenderProxyDataDirty();
}

void Decal::OnDestroyed()
{
	CoreObject::Destroy();
}

void Decal::OnTransformChanged(TransformChangedFlags flags)
{
	const Transform& transform = SceneObject()->GetLocalTransform();
	mWorldTransformMatrix = transform.GetMatrix();
	mWorldTransformMatrixWithoutScale = Matrix4::TRS(transform.GetPosition(), transform.GetRotation(), Vector3::kOne);

	MarkRenderProxyDataDirty(ComponentDirtyFlag::Transform);
}

SPtr<render::RenderProxy> Decal::CreateRenderProxy() const
{
	const SPtr<SceneInstance>& scene = SceneObject()->GetScene();

	render::Decal* renderProxy = new(B3DAllocate<render::Decal>()) render::Decal(B3DGetRenderProxy(scene));
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

RTTIType* Decal::GetRttiStatic()
{
	return DecalRTTI::Instance();
}

RTTIType* Decal::GetRtti() const
{
	return Decal::GetRttiStatic();
}

namespace b3d { namespace render
{
Decal::Decal(const SPtr<SceneInstance>& scene)
	: mSceneInstance(scene)
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
	RenderProxySyncPacket* const syncPacket = data.GetSyncPacket();
	if(syncPacket == nullptr)
		return;

	bool previousActiveState = mActive;
	syncPacket->ApplySyncData(this);

	mWorldTransformMatrix = mTransform.GetMatrix();
	mWorldTransformMatrixWithoutScale = Matrix4::TRS(mTransform.GetPosition(), mTransform.GetRotation(), Vector3::kOne);

	UpdateBounds();

	const SPtr<RendererScene>& rendererScene = mSceneInstance->GetRendererScene();

	const u32 flags = syncPacket->Flags;
	const u32 updateEverythingFlag = ~(u32)ComponentDirtyFlag::Transform;
	if((flags & updateEverythingFlag) != 0)
	{
		if(previousActiveState != mActive)
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
	else
	{
		if(mActive)
			rendererScene->UpdateDecal(this);
	}
}
}}
