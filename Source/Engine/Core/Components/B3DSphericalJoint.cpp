//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/B3DSphericalJoint.h"

#include "Physics/B3DPhysics.h"
#include "Scene/B3DSceneObject.h"
#include "RTTI/B3DSphericalJointRTTI.h"
#include "Scene/B3DSceneInstance.h"

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

void SphericalJoint::SetFlag(SphericalJointFlag flag, bool isEnabled)
{
	bool isFlagEnabled = mInformation.Flags.IsSet(flag);
	if(isFlagEnabled == isEnabled)
		return;

	if(isEnabled)
		mInformation.Flags.Set(flag);
	else
		mInformation.Flags.Unset(flag);

	if(mImplementation != nullptr)
		GetImplementation().SetFlag(flag, isEnabled);
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
