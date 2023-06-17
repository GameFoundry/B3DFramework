//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsDecal.h"
#include "Private/RTTI/BsDecalRTTI.h"
#include "RTTI/BsMathRTTI.h"
#include "Scene/BsSceneObject.h"
#include "Renderer/BsRenderer.h"
#include "Material/BsMaterial.h"
#include "CoreThread/BsCoreObjectSync.h"

using namespace bs;

DecalBase::DecalBase()
{
	UpdateBounds();
}

DecalBase::DecalBase(const Vector2& size, float maxDistance)
	: mSize(size), mMaxDistance(maxDistance)
{
	UpdateBounds();
}

void DecalBase::SetLayer(u64 layer)
{
	const bool isPow2 = layer && !((layer - 1) & layer);

	if(!isPow2)
	{
		B3D_LOG(Warning, Renderer, "Invalid layer provided. Only one layer bit may be set. Ignoring.");
		return;
	}

	mLayer = layer;
	MarkCoreDirtyInternal();
}

void DecalBase::SetTransform(const Transform& transform)
{
	if(mMobility != ObjectMobility::Movable)
		return;

	mTransform = transform;
	mTfrmMatrix = transform.GetMatrix();
	mTfrmMatrixNoScale = Matrix4::TRS(transform.GetPosition(), transform.GetRotation(), Vector3::kOne);

	MarkCoreDirtyInternal(ActorDirtyFlag::Transform);
}

void DecalBase::UpdateBounds()
{
	const Vector2& extents = mSize * 0.5f;

	AABox localAABB(
		Vector3(-extents.X, -extents.Y, -mMaxDistance),
		Vector3(extents.X, extents.Y, 0.0f));

	localAABB.TransformAffine(mTfrmMatrix);

	mBounds = Bounds(localAABB, Sphere(localAABB.GetCenter(), localAABB.GetRadius()));
}

template <bool Core>
template <class P>
void TDecal<Core>::RttiEnumFields(P p)
{
	p(mSize);
	p(mMaxDistance);
	p(mMaterial);
	p(mBounds);
	p(mLayer);
	p(mLayerMask);
}

Decal::Decal(const HMaterial& material, const Vector2& size, float maxDistance)
	: TDecal(material, size, maxDistance)
{
	// Calling virtual method is okay here because this is the most derived type
	UpdateBounds();
}

SPtr<ct::Decal> Decal::GetCore() const
{
	return std::static_pointer_cast<ct::Decal>(mCoreSpecific);
}

SPtr<Decal> Decal::Create(const HMaterial& material, const Vector2& size, float maxDistance)
{
	Decal* decal = new(B3DAllocate<Decal>()) Decal(material, size, maxDistance);
	SPtr<Decal> decalPtr = B3DMakeCoreFromExisting<Decal>(decal);
	decalPtr->SetShared(decalPtr);
	decalPtr->Initialize();

	return decalPtr;
}

SPtr<Decal> Decal::CreateEmpty()
{
	Decal* decal = new(B3DAllocate<Decal>()) Decal();
	SPtr<Decal> decalPtr = B3DMakeCoreFromExisting<Decal>(decal);
	decalPtr->SetShared(decalPtr);

	return decalPtr;
}

SPtr<ct::CoreObject> Decal::CreateCore() const
{
	SPtr<ct::Material> material;
	if(mMaterial.IsLoaded(false))
		material = mMaterial->GetCore();

	ct::Decal* decal = new(B3DAllocate<ct::Decal>()) ct::Decal(material, mSize, mMaxDistance);
	SPtr<ct::Decal> decalPtr = B3DMakeSharedFromExisting<ct::Decal>(decal);
	decalPtr->SetShared(decalPtr);

	return decalPtr;
}

void Decal::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	if(mMaterial.IsLoaded())
		dependencies.push_back(mMaterial.Get());
}

CoreSyncData Decal::SyncToCore(FrameAlloc* allocator)
{
	u32 size = 0;
	size += B3DRTTISize(GetCoreDirtyFlags()).Bytes;
	size += CoreSyncGetSize((SceneActor&)*this);
	size += CoreSyncGetSize(*this);

	u8* buffer = allocator->Alloc(size);

	Bitstream stream(buffer, size);
	B3DRTTIWrite(GetCoreDirtyFlags(), stream);
	B3DCoreSyncWrite((SceneActor&)*this, stream);
	B3DCoreSyncWrite(*this, stream);

	return CoreSyncData(buffer, size);
}

void Decal::MarkCoreDirtyInternal(ActorDirtyFlag flags)
{
	MarkCoreDirty((u32)flags);
}

RTTITypeBase* Decal::GetRttiStatic()
{
	return DecalRTTI::Instance();
}

RTTITypeBase* Decal::GetRtti() const
{
	return Decal::GetRttiStatic();
}

template class TDecal<true>;
template class TDecal<false>;

namespace bs { namespace ct
{
Decal::Decal(const SPtr<Material>& material, const Vector2& size, float maxDistance)
	: TDecal(material, size, maxDistance)
{}

Decal::~Decal()
{
	GetRenderer()->NotifyDecalRemoved(this);
}

void Decal::Initialize()
{
	UpdateBounds();
	GetRenderer()->NotifyDecalAdded(this);

	CoreObject::Initialize();
}

void Decal::SyncToCore(const CoreSyncData& data, FrameAlloc& allocator)
{
	Bitstream stream(data.GetBuffer(), data.GetBufferSize());

	u32 dirtyFlags = 0;
	bool oldIsActive = mActive;

	B3DRTTIRead(dirtyFlags, stream);
	B3DCoreSyncRead((SceneActor&)*this, stream);
	B3DCoreSyncRead(*this, stream);

	mTfrmMatrix = mTransform.GetMatrix();
	mTfrmMatrixNoScale = Matrix4::TRS(mTransform.GetPosition(), mTransform.GetRotation(), Vector3::kOne);

	UpdateBounds();

	if(dirtyFlags == (u32)ActorDirtyFlag::Transform)
	{
		if(mActive)
			GetRenderer()->NotifyDecalUpdated(this);
	}
	else
	{
		if(oldIsActive != mActive)
		{
			if(mActive)
				GetRenderer()->NotifyDecalAdded(this);
			else
				GetRenderer()->NotifyDecalRemoved(this);
		}
		else
		{
			GetRenderer()->NotifyDecalRemoved(this);
			GetRenderer()->NotifyDecalAdded(this);
		}
	}
}
}}
