//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsSphericalJoint.h"

#include "Physics/BsPhysics.h"
#include "Scene/BsSceneObject.h"
#include "Private/RTTI/BsSphericalJointRTTI.h"
#include "Scene/BsSceneInstance.h"

using namespace b3d;

SphericalJoint::SphericalJoint(const HSceneObject& parent)
	: Joint(parent, mInformation)
{
	SetName("SphericalJoint");
}

SphericalJoint::SphericalJoint()
	: SphericalJoint(nullptr)
{ }

void SphericalJoint::SetLimit(const LimitConeRange& limit)
{
	if(limit == mInformation.Limit)
		return;

	mInformation.Limit = limit;

	if(mImplementation != nullptr)
		GetImplementation().SetLimit(limit);
}

void SphericalJoint::SetFlag(SphericalJointFlag flag, bool enabled)
{
	bool isEnabled = mInformation.Flags.IsSet(flag);
	if(isEnabled == enabled)
		return;

	if(enabled)
		mInformation.Flags.Set(flag);
	else
		mInformation.Flags.Unset(flag);

	if(mImplementation != nullptr)
		GetImplementation().SetFlag(flag, enabled);
}

UPtr<IJointImplementation> SphericalJoint::CreateImplementation()
{
	const SPtr<SceneInstance>& scene = SO()->GetScene();
	return scene->GetPhysicsScene()->CreateSphericalJoint(*this, mInformation);
}

ISphericalJointImplementation& SphericalJoint::GetImplementation() const
{
	return static_cast<ISphericalJointImplementation&>(*mImplementation);
}

RTTIType* SphericalJoint::GetRttiStatic()
{
	return SphericalJointRTTI::Instance();
}

RTTIType* SphericalJoint::GetRtti() const
{
	return SphericalJoint::GetRttiStatic();
}
