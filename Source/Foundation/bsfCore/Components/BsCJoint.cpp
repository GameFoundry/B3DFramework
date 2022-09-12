//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCJoint.h"
#include "Components/BsCRigidbody.h"
#include "Scene/BsSceneObject.h"
#include "Physics/BsPhysics.h"
#include "Private/RTTI/BsCJointRTTI.h"

using namespace std::placeholders;

namespace bs
{
	CJoint::CJoint(JOINT_DESC& desc)
		:mDesc(desc)
	{
		mPositions[0] = Vector3::ZERO;
		mPositions[1] = Vector3::ZERO;

		mRotations[0] = Quaternion::IDENTITY;
		mRotations[1] = Quaternion::IDENTITY;
	}

	CJoint::CJoint(const HSceneObject& parent, JOINT_DESC& desc)
		: Component(parent), mDesc(desc)
	{
		setName("Joint");

		mPositions[0] = Vector3::ZERO;
		mPositions[1] = Vector3::ZERO;

		mRotations[0] = Quaternion::IDENTITY;
		mRotations[1] = Quaternion::IDENTITY;

		mNotifyFlags = (TransformChangedFlags)(TCF_Parent | TCF_Transform);
	}

	HRigidbody CJoint::GetBody(JointBody body) const
	{
		return mBodies[(int)body];
	}

	void CJoint::SetBody(JointBody body, const HRigidbody& value)
	{
		if (mBodies[(int)body] == value)
			return;

		if (mBodies[(int)body] != nullptr)
			mBodies[(int)body]->_setJoint(HJoint());

		mBodies[(int)body] = value;

		if (value != nullptr)
			mBodies[(int)body]->_setJoint(static_object_cast<CJoint>(mThisHandle));

		// If joint already exists, destroy it if we removed all bodies, otherwise update its transform
		if(mInternal != nullptr)
		{
			if (!isBodyValid(mBodies[0]) && !isBodyValid(mBodies[1]))
				destroyInternal();
			else
			{
				Rigidbody* rigidbody = nullptr;
				if (value != nullptr)
					rigidbody = value->_getInternal();

				mInternal->setBody(body, rigidbody);
				updateTransform(body);
			}
		}
		else // If joint doesn't exist, check if we can create it
		{
			// Must be an active component and at least one of the bodies must be non-null
			if (SO()->getActive() && (isBodyValid(mBodies[0]) || isBodyValid(mBodies[1])))
			{
				restoreInternal();
			}
		}
	}

	Vector3 CJoint::GetPosition(JointBody body) const
	{
		return mPositions[(int)body];
	}

	Quaternion CJoint::GetRotation(JointBody body) const
	{
		return mRotations[(int)body];
	}

	void CJoint::SetTransform(JointBody body, const Vector3& position, const Quaternion& rotation)
	{
		if (mPositions[(int)body] == position && mRotations[(int)body] == rotation)
			return;

		mPositions[(int)body] = position;
		mRotations[(int)body] = rotation;

		if (mInternal != nullptr)
			updateTransform(body);
	}

	float CJoint::GetBreakForce() const
	{
		return mDesc.breakForce;
	}

	void CJoint::SetBreakForce(float force)
	{
		if (mDesc.breakForce == force)
			return;

		mDesc.breakForce = force;

		if (mInternal != nullptr)
			mInternal->setBreakForce(force);
	}

	float CJoint::GetBreakTorque() const
	{
		return mDesc.breakTorque;
	}

	void CJoint::SetBreakTorque(float torque)
	{
		if (mDesc.breakTorque == torque)
			return;

		mDesc.breakTorque = torque;

		if (mInternal != nullptr)
			mInternal->setBreakTorque(torque);
	}

	bool CJoint::GetEnableCollision() const
	{
		return mDesc.enableCollision;
	}

	void CJoint::SetEnableCollision(bool value)
	{
		if (mDesc.enableCollision == value)
			return;

		mDesc.enableCollision = value;

		if (mInternal != nullptr)
			mInternal->setEnableCollision(value);
	}

	void CJoint::OnInitialized()
	{

	}

	void CJoint::OnDestroyed()
	{
		if (mBodies[0] != nullptr)
			mBodies[0]->_setJoint(HJoint());

		if (mBodies[1] != nullptr)
			mBodies[1]->_setJoint(HJoint());

		if(mInternal != nullptr)
			destroyInternal();
	}

	void CJoint::OnDisabled()
	{
		if (mInternal != nullptr)
			destroyInternal();
	}

	void CJoint::OnEnabled()
	{
		if(isBodyValid(mBodies[0]) || isBodyValid(mBodies[1]))
			restoreInternal();
	}

	void CJoint::OnTransformChanged(TransformChangedFlags flags)
	{
		if (mInternal == nullptr)
			return;

		// We're ignoring this during physics update because it would cause problems if the joint itself was moved by physics
		// Note: This isn't particularily correct because if the joint is being moved by physics but the rigidbodies
		// themselves are not parented to the joint, the transform will need updating. However I'm leaving it up to the
		// user to ensure rigidbodies are always parented to the joint in such a case (It's an unlikely situation that
		// I can't think of an use for - joint transform will almost always be set as an initialization step and not a
		// physics response).
		if (gPhysics()._isUpdateInProgress())
			return;

		updateTransform(JointBody::Target);
		updateTransform(JointBody::Anchor);
	}

	void CJoint::RestoreInternal()
	{
		if (mBodies[0] != nullptr)
			mDesc.bodies[0].body = mBodies[0]->_getInternal();
		else
			mDesc.bodies[0].body = nullptr;

		if (mBodies[1] != nullptr)
			mDesc.bodies[1].body = mBodies[1]->_getInternal();
		else
			mDesc.bodies[1].body = nullptr;

		getLocalTransform(JointBody::Target, mDesc.bodies[0].position, mDesc.bodies[0].rotation);
		getLocalTransform(JointBody::Anchor, mDesc.bodies[1].position, mDesc.bodies[1].rotation);

		mInternal = createInternal();

		mInternal->onJointBreak.connect(std::bind(&CJoint::triggerOnJointBroken, this));
	}

	void CJoint::DestroyInternal()
	{
		// This should release the last reference and destroy the internal joint
		if(mInternal)
		{
			mInternal->_setOwner(PhysicsOwnerType::None, nullptr);
			mInternal = nullptr;
		}
	}

	void CJoint::NotifyRigidbodyMoved(const HRigidbody& body)
	{
		if (mInternal == nullptr)
			return;

		// If physics update is in progress do nothing, as its the joint itself that's probably moving the body
		if (gPhysics()._isUpdateInProgress())
			return;

		if (mBodies[0] == body)
			updateTransform(JointBody::Target);
		else if (mBodies[1] == body)
			updateTransform(JointBody::Anchor);
		else
			assert(false); // Not allowed to happen
	}

	bool CJoint::IsBodyValid(const HRigidbody& body)
	{
		if (body == nullptr)
			return false;

		if (body->_getInternal() == nullptr)
			return false;

		return true;
	}

	void CJoint::UpdateTransform(JointBody body)
	{
		Vector3 localPos;
		Quaternion localRot;
		getLocalTransform(body, localPos, localRot);

		mInternal->setTransform(body, localPos, localRot);
	}

	void CJoint::GetLocalTransform(JointBody body, Vector3& position, Quaternion& rotation)
	{
		position = mPositions[(UINT32)body];
		rotation = mRotations[(UINT32)body];

		HRigidbody rigidbody = mBodies[(UINT32)body];
		if (rigidbody == nullptr) // Get world space transform if no relative to any body
		{
			const Transform& tfrm = SO()->getTransform();
			Quaternion worldRot = tfrm.getRotation();

			rotation = worldRot*rotation;
			position = worldRot.rotate(position) + tfrm.getPosition();
		}
		else
		{
			position = rotation.rotate(position);
		}
	}
	
	void CJoint::TriggerOnJointBroken()
	{
		onJointBreak();
	}

	RTTITypeBase* CJoint::getRTTIStatic()
	{
		return CJointRTTI::Instance();
	}

	RTTITypeBase* CJoint::getRTTI() const
	{
		return CJoint::GetRTTIStatic();
	}
}
