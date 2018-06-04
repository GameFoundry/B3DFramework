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

	char* SceneActor::syncActorTo(char* data)
	{
		data = rttiWriteElem(mTransform.getPosition(), data);
		data = rttiWriteElem(mTransform.getRotation(), data);
		data = rttiWriteElem(mTransform.getScale(), data);
		data = rttiWriteElem(mActive, data);
		data = rttiWriteElem(mMobility, data);
		
		return data;
	}

	char* SceneActor::syncActorFrom(char* data)
	{
		Vector3 position;
		Quaternion rotation;
		Vector3 scale;

		data = rttiReadElem(position, data);
		data = rttiReadElem(rotation, data);
		data = rttiReadElem(scale, data);
		data = rttiReadElem(mActive, data);
		data = rttiReadElem(mMobility, data);

		mTransform.setPosition(position);
		mTransform.setRotation(rotation);
		mTransform.setScale(scale);

		return data;
	}

	UINT32 SceneActor::getActorSyncDataSize() const
	{
		UINT32 size =
			rttiGetElemSize(mTransform.getPosition()) +
			rttiGetElemSize(mTransform.getRotation()) +
			rttiGetElemSize(mTransform.getScale()) +
			rttiGetElemSize(mActive) +
			rttiGetElemSize(mMobility);

		return size;
	}
}
