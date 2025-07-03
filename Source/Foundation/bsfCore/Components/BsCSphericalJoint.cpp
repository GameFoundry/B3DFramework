//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCSphericalJoint.h"
#include "Scene/BsSceneObject.h"
#include "Private/RTTI/BsCSphericalJointRTTI.h"
#include "Scene/BsSceneInstance.h"

using namespace b3d;

CSphericalJoint::CSphericalJoint()
	: CJoint(mDesc)
{
	SetName("SphericalJoint");
}

CSphericalJoint::CSphericalJoint(const HSceneObject& parent)
	: CJoint(parent, mDesc)
{
	SetName("SphericalJoint");
}

LimitConeRange CSphericalJoint::GetLimit() const
{
	return mDesc.Limit;
}

void CSphericalJoint::SetLimit(const LimitConeRange& limit)
{
	if(limit == mDesc.Limit)
		return;

	mDesc.Limit = limit;

	if(mInternal != nullptr)
		GetInternalInternal()->SetLimit(limit);
}

void CSphericalJoint::SetFlag(SphericalJointFlag flag, bool enabled)
{
	bool isEnabled = ((u32)mDesc.Flag & (u32)flag) != 0;
	if(isEnabled == enabled)
		return;

	if(enabled)
		mDesc.Flag = (SphericalJointFlag)((u32)mDesc.Flag | (u32)flag);
	else
		mDesc.Flag = (SphericalJointFlag)((u32)mDesc.Flag & ~(u32)flag);

	if(mInternal != nullptr)
		GetInternalInternal()->SetFlag(flag, enabled);
}

bool CSphericalJoint::HasFlag(SphericalJointFlag flag) const
{
	return ((u32)mDesc.Flag & (u32)flag) != 0;
}

SPtr<Joint> CSphericalJoint::CreateInternal()
{
	const SPtr<SceneInstance>& scene = SO()->GetScene();
	SPtr<Joint> joint = SphericalJoint::Create(*scene->GetPhysicsScene(), mDesc);

	joint->SetOwnerInternal(PhysicsOwnerType::Component, this);
	return joint;
}

RTTIType* CSphericalJoint::GetRttiStatic()
{
	return CSphericalJointRTTI::Instance();
}

RTTIType* CSphericalJoint::GetRtti() const
{
	return CSphericalJoint::GetRttiStatic();
}
