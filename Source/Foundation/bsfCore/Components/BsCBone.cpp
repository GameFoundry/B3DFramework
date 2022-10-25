//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCBone.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCAnimation.h"
#include "Private/RTTI/BsCBoneRTTI.h"

using namespace std::placeholders;

using namespace bs;

CBone::CBone()
{
	SetName("Bone");

	mNotifyFlags = TCF_Parent;
	SetFlag(ComponentFlag::AlwaysRun, true);
}

CBone::CBone(const HSceneObject& parent)
	: Component(parent)
{
	SetName("Bone");

	mNotifyFlags = TCF_Parent;
}

void CBone::SetBoneName(const String& name)
{
	if(mBoneName == name)
		return;

	mBoneName = name;

	if(mParent != nullptr)
		mParent->NotifyBoneChangedInternal(static_object_cast<CBone>(GetHandle()));
}

void CBone::OnDestroyed()
{
	if(mParent != nullptr)
		mParent->RemoveBoneInternal(static_object_cast<CBone>(GetHandle()));

	mParent = nullptr;
}

void CBone::OnDisabled()
{
	if(mParent != nullptr)
		mParent->RemoveBoneInternal(static_object_cast<CBone>(GetHandle()));

	mParent = nullptr;
}

void CBone::OnEnabled()
{
	UpdateParentAnimation();
}

void CBone::OnTransformChanged(TransformChangedFlags flags)
{
	if(!SO()->GetActive())
		return;

	if((flags & TCF_Parent) != 0)
		UpdateParentAnimation();
}

void CBone::UpdateParentAnimation()
{
	HSceneObject currentSO = SO();
	while(currentSO != nullptr)
	{
		HAnimation parent = currentSO->GetComponent<CAnimation>();
		if(parent != nullptr)
		{
			if(currentSO->GetActive())
				SetParentInternal(parent);
			else
				SetParentInternal(HAnimation());

			return;
		}

		currentSO = currentSO->GetParent();
	}

	SetParentInternal(HAnimation());
}

void CBone::SetParentInternal(const HAnimation& animation, bool isInternal)
{
	if(animation == mParent)
		return;

	if(!isInternal)
	{
		if(mParent != nullptr)
			mParent->RemoveBoneInternal(static_object_cast<CBone>(GetHandle()));

		if(animation != nullptr)
			animation->AddBoneInternal(static_object_cast<CBone>(GetHandle()));
	}

	mParent = animation;
}

RTTITypeBase* CBone::GetRttiStatic()
{
	return CBoneRTTI::Instance();
}

RTTITypeBase* CBone::GetRtti() const
{
	return CBone::GetRttiStatic();
}
