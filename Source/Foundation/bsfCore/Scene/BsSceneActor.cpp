//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsSceneActor.h"
#include "CoreObject/BsCoreObjectSync.h"
#include "Scene/BsSceneObject.h"

using namespace bs;

namespace bs
{
	B3D_SYNC_BLOCK_BEGIN(SceneActor, SyncEverythingPacket)
		B3D_SYNC_BLOCK_ENTRY(mTransform)
		B3D_SYNC_BLOCK_ENTRY(mMobility)
		B3D_SYNC_BLOCK_ENTRY(mActive)
	B3D_SYNC_BLOCK_END

	B3D_SYNC_BLOCK_BEGIN(SceneActor, SyncTransformPacket)
		B3D_SYNC_BLOCK_ENTRY(mTransform)
	B3D_SYNC_BLOCK_END

	B3D_SYNC_BLOCK_BEGIN(SceneActor, SyncActiveStatePacket)
		B3D_SYNC_BLOCK_ENTRY(mActive)
	B3D_SYNC_BLOCK_END
}

void SceneActor::SetTransform(const Transform& transform)
{
	if(mMobility != ObjectMobility::Movable)
		return;

	mTransform = transform;
	MarkCoreDirtyInternal(ActorDirtyFlag::Transform);
}

void SceneActor::SetMobility(ObjectMobility mobility)
{
	mMobility = mobility;
	MarkCoreDirtyInternal(ActorDirtyFlag::Mobility);
}

void SceneActor::SetActive(bool active)
{
	mActive = active;
	MarkCoreDirtyInternal(ActorDirtyFlag::Active);
}

void SceneActor::UpdateStateInternal(const SceneObject& so, bool force)
{
	u32 curHash = so.GetTransformHash();
	if(curHash != mHash || force)
	{
		SetTransform(so.GetTransform());

		mHash = curHash;
	}

	if(so.GetActive() != mActive || force)
		SetActive(so.GetActive());

	if(so.GetMobility() != mMobility || force)
		SetMobility(so.GetMobility());
}

RenderProxySyncPacket* SceneActor::CreateSceneActorRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	if(flags == 0)
		return nullptr;

	ActorDirtyFlags actorFlags(flags);
	actorFlags &= (ActorDirtyFlag::Active | ActorDirtyFlag::Mobility | ActorDirtyFlag::Everything | ActorDirtyFlag::Mobility);

	if(actorFlags == ActorDirtyFlag::Transform)
		return allocator.Construct<SyncTransformPacket>(*this, allocator, flags);

	if(actorFlags == ActorDirtyFlag::Active)
		return allocator.Construct<SyncActiveStatePacket>(*this, allocator, flags);

	return allocator.Construct<SyncEverythingPacket>(*this, allocator, flags);
}
