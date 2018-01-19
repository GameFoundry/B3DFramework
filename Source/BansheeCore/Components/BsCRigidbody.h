//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsRigidbody.h"
#include "Scene/BsComponent.h"
#include "Math/BsVector3.h"
#include "Math/BsQuaternion.h"

namespace bs
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	Rigidbody
	 *
	 * Colliders that are on the same scene object as the rigidbody, or on child scene objects are automatically considered
	 * as part of the rigidbody.
	 *
	 * @note Wraps Rigidbody as a Component.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Physics,n:Rigidbody) CRigidbody : public Component
	{
	public:
		CRigidbody(const HSceneObject& parent);

		/** @copydoc Rigidbody::move */
		BS_SCRIPT_EXPORT(n:Move)
		void move(const Vector3& position);

		/** @copydoc Rigidbody::rotate */
		BS_SCRIPT_EXPORT(n:Rotate)
		void rotate(const Quaternion& rotation);

		/** @copydoc Rigidbody::setMass */
		BS_SCRIPT_EXPORT(n:Mass,pr:setter)
		void setMass(float mass);

		/** @copydoc Rigidbody::getMass */
		BS_SCRIPT_EXPORT(n:Mass,pr:getter)
		float getMass() const { return mMass; };

		/** @copydoc Rigidbody::setIsKinematic */
		BS_SCRIPT_EXPORT(n:IsKinematic,pr:setter)
		void setIsKinematic(bool kinematic);

		/** @copydoc Rigidbody::getIsKinematic */
		BS_SCRIPT_EXPORT(n:IsKinematic,pr:getter)
		bool getIsKinematic() const { return mIsKinematic; }

		/** @copydoc Rigidbody::isSleeping */
		BS_SCRIPT_EXPORT(n:IsSleeping,pr:getter)
		bool isSleeping() const;

		/** @copydoc Rigidbody::sleep */
		BS_SCRIPT_EXPORT(n:Sleep)
		void sleep();

		/** @copydoc Rigidbody::wakeUp */
		BS_SCRIPT_EXPORT(n:WakeUp)
		void wakeUp();

		/** @copydoc Rigidbody::setSleepThreshold */
		BS_SCRIPT_EXPORT(n:SleepThreshold,pr:setter)
		void setSleepThreshold(float threshold);

		/** @copydoc Rigidbody::getSleepThreshold */
		BS_SCRIPT_EXPORT(n:SleepThreshold,pr:getter)
		float getSleepThreshold() const { return mSleepThreshold; }

		/** @copydoc Rigidbody::setUseGravity */
		BS_SCRIPT_EXPORT(n:UseGravity,pr:setter)
		void setUseGravity(bool gravity);

		/** @copydoc Rigidbody::getUseGravity */
		BS_SCRIPT_EXPORT(n:UseGravity,pr:getter)
		bool getUseGravity() const { return mUseGravity; }

		/** @copydoc Rigidbody::setVelocity */
		BS_SCRIPT_EXPORT(n:Velocity,pr:setter)
		void setVelocity(const Vector3& velocity);

		/** @copydoc Rigidbody::getVelocity */
		BS_SCRIPT_EXPORT(n:Velocity,pr:getter)
		Vector3 getVelocity() const;

		/** @copydoc Rigidbody::setAngularVelocity */
		BS_SCRIPT_EXPORT(n:AngularVelocity,pr:setter)
		void setAngularVelocity(const Vector3& velocity);

		/** @copydoc Rigidbody::getAngularVelocity */
		BS_SCRIPT_EXPORT(n:AngularVelocity,pr:getter)
		Vector3 getAngularVelocity() const;

		/** @copydoc Rigidbody::setDrag */
		BS_SCRIPT_EXPORT(n:Drag,pr:setter)
		void setDrag(float drag);

		/** @copydoc Rigidbody::getDrag */
		BS_SCRIPT_EXPORT(n:Drag,pr:getter)
		float getDrag() const { return mLinearDrag; }

		/** @copydoc Rigidbody::setAngularDrag */
		BS_SCRIPT_EXPORT(n:AngularDrag,pr:setter)
		void setAngularDrag(float drag);

		/** @copydoc Rigidbody::getAngularDrag */
		BS_SCRIPT_EXPORT(n:AngularDrag,pr:getter)
		float getAngularDrag() const { return mAngularDrag; }

		/** @copydoc Rigidbody::setInertiaTensor */
		BS_SCRIPT_EXPORT(n:InertiaTensor,pr:setter)
		void setInertiaTensor(const Vector3& tensor);

		/** @copydoc Rigidbody::getInertiaTensor */
		BS_SCRIPT_EXPORT(n:InertiaTensor,pr:getter)
		Vector3 getInertiaTensor() const;

		/** @copydoc Rigidbody::setMaxAngularVelocity */
		BS_SCRIPT_EXPORT(n:MaxAngularVelocity,pr:setter)
		void setMaxAngularVelocity(float maxVelocity);

		/** @copydoc Rigidbody::getMaxAngularVelocity */
		BS_SCRIPT_EXPORT(n:MaxAngularVelocity,pr:getter)
		float getMaxAngularVelocity() const { return mMaxAngularVelocity; }

		/** Determines the rigidbody's center of mass position. Only relevant if RigibodyFlag::AutoTensors is turned off. */
		BS_SCRIPT_EXPORT(n:CenterOfMassPosition,pr:setter)
		void setCenterOfMassPosition(const Vector3& position);

		/** @copydoc setCenterOfMassPosition() */
		BS_SCRIPT_EXPORT(n:CenterOfMassPosition,pr:getter)
		Vector3 getCenterOfMassPosition() const;

		/** Determines the rigidbody's center of mass rotation. Only relevant if RigibodyFlag::AutoTensors is turned off. */
		BS_SCRIPT_EXPORT(n:CenterOfMassRotation,pr:setter)
		void setCenterOfMassRotation(const Quaternion& rotation);

		/** @copydoc setCenterOfMassRotation() */
		BS_SCRIPT_EXPORT(n:CenterOfMassRotation,pr:getter)
		Quaternion getCenterOfMassRotation() const;

		/** @copydoc Rigidbody::setPositionSolverCount */
		BS_SCRIPT_EXPORT(n:PositionSolverCount,pr:setter)
		void setPositionSolverCount(UINT32 count);

		/** @copydoc Rigidbody::getPositionSolverCount */
		BS_SCRIPT_EXPORT(n:PositionSolverCount,pr:getter)
		UINT32 getPositionSolverCount() const { return mPositionSolverCount; }

		/** @copydoc Rigidbody::setVelocitySolverCount */
		BS_SCRIPT_EXPORT(n:VelocitySolverCount,pr:setter)
		void setVelocitySolverCount(UINT32 count);

		/** @copydoc Rigidbody::getVelocitySolverCount */
		BS_SCRIPT_EXPORT(n:VelocitySolverCount,pr:getter)
		UINT32 getVelocitySolverCount() const { return mVelocitySolverCount; }

		/** Sets a value that determines which (if any) collision events are reported. */
		BS_SCRIPT_EXPORT(n:CollisionReportMode,pr:setter)
		void setCollisionReportMode(CollisionReportMode mode);

		/** Gets a value that determines which (if any) collision events are reported. */
		BS_SCRIPT_EXPORT(n:CollisionReportMode,pr:getter)
		CollisionReportMode getCollisionReportMode() const { return mCollisionReportMode; }

		/** @copydoc Rigidbody::setFlags */
		BS_SCRIPT_EXPORT(n:Flags,pr:setter)
		void setFlags(RigidbodyFlag flags);

		/** @copydoc Rigidbody::getFlags */
		BS_SCRIPT_EXPORT(n:Flags,pr:getter)
		RigidbodyFlag getFlags() const { return mFlags; }

		/** @copydoc Rigidbody::addForce */
		BS_SCRIPT_EXPORT(n:AddForce)
		void addForce(const Vector3& force, ForceMode mode = ForceMode::Force);

		/** @copydoc Rigidbody::addTorque */
		BS_SCRIPT_EXPORT(n:AddTorque)
		void addTorque(const Vector3& torque, ForceMode mode = ForceMode::Force);

		/** @copydoc Rigidbody::addForceAtPoint */
		BS_SCRIPT_EXPORT(n:AddForceAtPoint)
		void addForceAtPoint(const Vector3& force, const Vector3& position,
			PointForceMode mode = PointForceMode::Force);

		/** @copydoc Rigidbody::getVelocityAtPoint */
		BS_SCRIPT_EXPORT(n:GetVelocityAtPoint)
		Vector3 getVelocityAtPoint(const Vector3& point) const;

		/** @copydoc Rigidbody::onCollisionBegin */
		BS_SCRIPT_EXPORT(n:OnCollisionBegin)
		Event<void(const CollisionData&)> onCollisionBegin;

		/** @copydoc Rigidbody::onCollisionStay */
		BS_SCRIPT_EXPORT(n:OnCollisionStay)
		Event<void(const CollisionData&)> onCollisionStay;

		/** @copydoc Rigidbody::onCollisionEnd */
		BS_SCRIPT_EXPORT(n:OnCollisionEnd)
		Event<void(const CollisionData&)> onCollisionEnd;

		/** @name Internal
		 *  @{
		 */

		/** Returns the Rigidbody implementation wrapped by this component. */
		Rigidbody* _getInternal() const { return mInternal.get(); }

		/** Sets that joint that this rigidbody is attached to. Allows the rigidbody to notify the joint when it moves. */
		void _setJoint(const HJoint& joint) { mParentJoint = joint; }

		/** @copydoc Rigidbody::updateMassDistribution */
		void _updateMassDistribution();

		/** @} */
	protected:
		friend class CCollider;

		using Component::destroyInternal;

		/** 
		 * Searches child scene objects for Collider components and attaches them to the rigidbody. Make sure to call
		 * clearColliders() if you need to clear old colliders first. 
		 */
		void updateColliders();

		/** Unregisters all child colliders from the Rigidbody. */
		void clearColliders();

		/** 
		 * Registers a new collider with the Rigidbody. This collider will then be used to calculate Rigidbody's geometry
		 * used for collisions, and optionally (depending on set flags) total mass, inertia tensors and center of mass.
		 */
		void addCollider(const HCollider& collider);

		/** Unregisters the collider from the Rigidbody. */
		void removeCollider(const HCollider& collider);

		/** Checks if the rigidbody is nested under another rigidbody, and throws out a warning if so. */
		void checkForNestedRigibody();

		/** Appends Component referenes for the colliders to the collision data. */
		void processCollisionData(const CollisionDataRaw& raw, CollisionData& output);

		/** Destroys the internal rigidbody representation. */
		void destroyInternal();

		/** Triggered when the internal rigidbody begins touching another object. */
		void triggerOnCollisionBegin(const CollisionDataRaw& data);

		/** Triggered when the internal rigidbody continues touching another object. */
		void triggerOnCollisionStay(const CollisionDataRaw& data);

		/** Triggered when the internal rigidbody ends touching another object. */
		void triggerOnCollisionEnd(const CollisionDataRaw& data);

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		/** @copydoc Component::onInitialized() */
		void onInitialized() override;

		/** @copydoc Component::onDestroyed() */
		void onDestroyed() override;

		/** @copydoc Component::onDisabled() */
		void onDisabled() override;

		/** @copydoc Component::onEnabled() */
		void onEnabled() override;

		/** @copydoc Component::onTransformChanged() */
		void onTransformChanged(TransformChangedFlags flags) override;

		SPtr<Rigidbody> mInternal;
		Vector<HCollider> mChildren;
		HJoint mParentJoint;

		UINT32 mPositionSolverCount = 4;
		UINT32 mVelocitySolverCount = 1;
		RigidbodyFlag mFlags = (RigidbodyFlag)((UINT32)RigidbodyFlag::AutoTensors | (UINT32)RigidbodyFlag::AutoMass);
		CollisionReportMode mCollisionReportMode = CollisionReportMode::None;
		Vector3 mCMassPosition = Vector3::ZERO;
		Quaternion mCMassRotation = Quaternion::IDENTITY;
		Vector3 mInertiaTensor = Vector3::ZERO;
		float mMass = 0.0f;
		float mMaxAngularVelocity = std::numeric_limits<float>::max();
		float mLinearDrag = 0.0f;
		float mAngularDrag = 0.0f;
		float mSleepThreshold = 0.0f;
		bool mUseGravity = true;
		bool mIsKinematic = false;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CRigidbodyRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		CRigidbody(); // Serialization only
	};

	/** @} */
}