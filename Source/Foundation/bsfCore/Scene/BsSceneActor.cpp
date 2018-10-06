//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsSceneActor.h"
#include "Scene/BsSceneObject.h"

namespace bs
{
	void SceneActor::setTransform(const Transform& transform)
	{
		if (mMobility != ObjectMobility::Movable)
			return;

		mTransform = transform;
		_markCoreDirty(ActorDirtyFlag::Transform);
	}

	void SceneActor::setMobility(ObjectMobility mobility)
	{
		mMobility = mobility;
		_markCoreDirty(ActorDirtyFlag::Mobility);
	}

	void SceneActor::setActive(bool active)
	{
		mActive = active;
		_markCoreDirty(ActorDirtyFlag::Active);
	}

	void SceneActor::_updateState(const SceneObject& so, bool force)
	{
		UINT32 curHash = so.getTransformHash();
		if (curHash != mHash || force)
		{
			setTransform(so.getTransform());

			mHash = curHash;
		}

		if (so.getActive() != mActive || force)
			setActive(so.getActive());

		if (so.getMobility() != mMobility || force)
			setMobility(so.getMobility());
	}
}
