//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCBone.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCAnimation.h"
#include "Private/RTTI/BsCBoneRTTI.h"

using namespace std::placeholders;

namespace bs
{
	CBone::CBone()
	{
		setName("Bone");

		mNotifyFlags = TCF_Parent;
		setFlag(ComponentFlag::AlwaysRun, true);
	}

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