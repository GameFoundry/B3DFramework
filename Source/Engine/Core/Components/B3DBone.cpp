//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/B3DBone.h"
#include "Scene/B3DSceneObject.h"
#include "Components/B3DAnimation.h"
#include "RTTI/B3DBoneRTTI.h"

using namespace b3d;

Bone::Bone(const HSceneObject& parent)
	: Component(parent)
{
	SetName("Bone");

	mNotifyFlags = TCF_Parent;
}

Bone::Bone()
	: Bone(nullptr)
{ }

void Bone::SetBoneName(const String& name)
{
	if(mBoneName == name)
		return;

	mBoneName = name;

	if(mParent != nullptr)
		mParent->NotifyBoneNameChanged(B3DStaticGameObjectCast<Bone>(GetHandle()));
}

void Bone::OnDestroyed()
{
	if(mParent != nullptr)
		mParent->RemoveBone(B3DStaticGameObjectCast<Bone>(GetHandle()));

	mParent = nullptr;
}

void Bone::OnDisabled()
{
	if(mParent != nullptr)
		mParent->RemoveBone(B3DStaticGameObjectCast<Bone>(GetHandle()));

	mParent = nullptr;
}

void Bone::OnEnabled()
{
	UpdateParentAnimation();
}

void Bone::OnTransformChanged(TransformChangedFlags flags)
{
	if(!GetEnabled())
		return;

	if((flags & TCF_Parent) != 0)
		UpdateParentAnimation();
}

void Bone::UpdateParentAnimation()
{
	HSceneObject currentSceneObject = SO();
	while(currentSceneObject != nullptr)
	{
		HAnimation parent = currentSceneObject->GetComponent<Animation>();
		if(parent != nullptr)
		{
			if(parent->GetEnabled())
				SetParentAnimation(parent);
			else
				SetParentAnimation(HAnimation());

			return;
		}

		currentSceneObject = currentSceneObject->GetParent();
	}

	SetParentAnimation(HAnimation());
}

void Bone::SetParentAnimation(const HAnimation& animation, bool isInternal)
{
	if(animation == mParent)
		return;

	if(!isInternal)
	{
		if(mParent != nullptr)
			mParent->RemoveBone(B3DStaticGameObjectCast<Bone>(GetHandle()));

		if(animation != nullptr)
			animation->AddBone(B3DStaticGameObjectCast<Bone>(GetHandle()));
	}

	mParent = animation;
}

RTTIType* Bone::GetRttiStatic()
{
	return BoneRTTI::Instance();
}

RTTIType* Bone::GetRtti() const
{
	return Bone::GetRttiStatic();
}
