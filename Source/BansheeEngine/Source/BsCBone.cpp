//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCBone.h"
#include "BsSceneObject.h"
#include "BsCAnimation.h"
#include "BsCBoneRTTI.h"

using namespace std::placeholders;

namespace bs
{
	CBone::CBone(const HSceneObject& parent)
		: Component(parent)
	{
		setName("Bone");

		mNotifyFlags = TCF_Parent;
	}

	void CBone::setBoneName(const String& name)
	{
		if (mBoneName == name)
			return;

		mBoneName = name;

		if (mParent != nullptr)
			mParent->_notifyBoneChanged(getHandle());
	}

	void CBone::onDestroyed()
	{
		if (mParent != nullptr)
			mParent->_removeBone(getHandle());

		mParent = nullptr;
	}

	void CBone::onDisabled()
	{
		if (mParent != nullptr)
			mParent->_removeBone(getHandle());

		mParent = nullptr;
	}

	void CBone::onEnabled()
	{
		updateParentAnimation();
	}

	void CBone::onTransformChanged(TransformChangedFlags flags)
	{
		if (!SO()->getActive())
			return;

		if ((flags & TCF_Parent) != 0)
			updateParentAnimation();
	}

	void CBone::updateParentAnimation()
	{
		HSceneObject currentSO = SO();
		while (currentSO != nullptr)
		{
			HAnimation parent = currentSO->getComponent<CAnimation>();
			if (parent != nullptr)
			{
				if (currentSO->getActive())
					_setParent(parent);
				else
					_setParent(HAnimation());

				return;
			}

			currentSO = currentSO->getParent();
		}

		_setParent(HAnimation());
	}

	void CBone::_setParent(const HAnimation& animation, bool isInternal)
	{
		if (animation == mParent)
			return;

		if (!isInternal)
		{
			if (mParent != nullptr)
				mParent->_removeBone(getHandle());

			if (animation != nullptr)
				animation->_addBone(getHandle());
		}

		mParent = animation;
	}
	
	RTTITypeBase* CBone::getRTTIStatic()
	{
		return CBoneRTTI::instance();
	}

	RTTITypeBase* CBone::getRTTI() const
	{
		return CBone::getRTTIStatic();
	}
}