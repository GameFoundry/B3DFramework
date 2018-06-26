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

	char* SceneActor::syncActorTo(char* data, ActorDirtyFlags flags)
	{
		if(flags.isSetAny(ActorDirtyFlag::Transform | ActorDirtyFlag::Everything))
		{
			data = rttiWriteElem(mTransform.getPosition(), data);
			data = rttiWriteElem(mTransform.getRotation(), data);
			data = rttiWriteElem(mTransform.getScale(), data);
		}

		if(flags.isSetAny(ActorDirtyFlag::Active | ActorDirtyFlag::Everything))
			data = rttiWriteElem(mActive, data);

		if(flags.isSetAny(ActorDirtyFlag::Mobility | ActorDirtyFlag::Everything))
			data = rttiWriteElem(mMobility, data);
		
		return data;
	}

	char* SceneActor::syncActorFrom(char* data, ActorDirtyFlags flags)
	{
		if(flags.isSetAny(ActorDirtyFlag::Transform | ActorDirtyFlag::Everything))
		{
			Vector3 position;
			Quaternion rotation;
			Vector3 scale;

			data = rttiReadElem(position, data);
			data = rttiReadElem(rotation, data);
			data = rttiReadElem(scale, data);

			mTransform.setPosition(position);
			mTransform.setRotation(rotation);
			mTransform.setScale(scale);
		}

		if(flags.isSetAny(ActorDirtyFlag::Active | ActorDirtyFlag::Everything))
			data = rttiReadElem(mActive, data);

		if(flags.isSetAny(ActorDirtyFlag::Mobility | ActorDirtyFlag::Everything))
			data = rttiReadElem(mMobility, data);

		return data;
	}

	UINT32 SceneActor::getActorSyncDataSize(ActorDirtyFlags flags) const
	{
		UINT32 size = 0;

		if(flags.isSetAny(ActorDirtyFlag::Transform | ActorDirtyFlag::Everything))
		{
			size = rttiGetElemSize(mTransform.getPosition()) +
				rttiGetElemSize(mTransform.getRotation()) +
				rttiGetElemSize(mTransform.getScale());
		}

		if(flags.isSetAny(ActorDirtyFlag::Active | ActorDirtyFlag::Everything))
			size += rttiGetElemSize(mActive);

		if(flags.isSetAny(ActorDirtyFlag::Mobility | ActorDirtyFlag::Everything))
			size += rttiGetElemSize(mMobility);

		return size;
	}
}
