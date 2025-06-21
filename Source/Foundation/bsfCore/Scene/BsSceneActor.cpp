//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsSceneActor.h"
#include "CoreObject/BsCoreObjectSync.h"
#include "Scene/BsSceneObject.h"

using namespace b3d;

namespace b3d
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

void SceneActor::UpdateStateFromSceneObject(const SceneObject& so, bool force)
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

	const SPtr<SceneInstance>& sceneInstance = so.GetScene();
	if(mSceneInstance != sceneInstance || force)
		SetScene(sceneInstance);
}

RenderProxySyncPacket* SceneActor::CreateSceneActorRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	if(flags == 0)
		return nullptr;

	ActorDirtyFlags actorFlags(flags);
	actorFlags &= (ActorDirtyFlag::Transform | ActorDirtyFlag::Mobility | ActorDirtyFlag::Active | ActorDirtyFlag::Everything);

	if(actorFlags == ActorDirtyFlag::Transform)
		return allocator.Construct<SyncTransformPacket>(*this, allocator, flags);

	if(actorFlags == ActorDirtyFlag::Active)
		return allocator.Construct<SyncActiveStatePacket>(*this, allocator, flags);

	return allocator.Construct<SyncEverythingPacket>(*this, allocator, flags);
}
