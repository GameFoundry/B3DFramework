//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsSceneActor.h"
#include "Scene/BsSceneObject.h"

namespace bs
{
	void SceneActor::SetTransform(const Transform& transform)
	{
		if (mMobility != ObjectMobility::Movable)
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
		UINT32 curHash = so.GetTransformHash();
		if (curHash != mHash || force)
		{
			SetTransform(so.GetTransform());

			mHash = curHash;
		}

		if (so.GetActive() != mActive || force)
			SetActive(so.GetActive());

		if (so.GetMobility() != mMobility || force)
			SetMobility(so.GetMobility());
	}
}
