//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
		void Move(const Vector3& position);

		/** @copydoc Rigidbody::rotate */
		BS_SCRIPT_EXPORT(n:Rotate)
		void Rotate(const Quaternion& rotation);

		/** @copydoc Rigidbody::setMass */
		BS_SCRIPT_EXPORT(n:Mass,pr:setter)
		void SetMass(float mass);

		/** @copydoc Rigidbody::getMass */
		BS_SCRIPT_EXPORT(n:Mass,pr:getter)
		float GetMass() const { return mMass; };

		/** @copydoc Rigidbody::setIsKinematic */
		BS_SCRIPT_EXPORT(n:IsKinematic,pr:setter)
		void SetIsKinematic(bool kinematic);

		/** @copydoc Rigidbody::getIsKinematic */
		BS_SCRIPT_EXPORT(n:IsKinematic,pr:getter)
		bool GetIsKinematic() const { return mIsKinematic; }

		/** @copydoc Rigidbody::isSleeping */
		BS_SCRIPT_EXPORT(n:IsSleeping,pr:getter)
		bool IsSleeping() const;

		/** @copydoc Rigidbody::sleep */
		BS_SCRIPT_EXPORT(n:Sleep)
		void Sleep();

		/** @copydoc Rigidbody::wakeUp */
		BS_SCRIPT_EXPORT(n:WakeUp)
		void WakeUp();

		/** @copydoc Rigidbody::setSleepThreshold */
		BS_SCRIPT_EXPORT(n:SleepThreshold,pr:setter,hide)
		void SetSleepThreshold(float threshold);

		/** @copydoc Rigidbody::getSleepThreshold */
		BS_SCRIPT_EXPORT(n:SleepThreshold,pr:getter)
		float GetSleepThreshold() const { return mSleepThreshold; }

		/** @copydoc Rigidbody::setUseGravity */
		BS_SCRIPT_EXPORT(n:UseGravity,pr:setter)
		void SetUseGravity(bool gravity);

		/** @copydoc Rigidbody::getUseGravity */
		BS_SCRIPT_EXPORT(n:UseGravity,pr:getter)
		bool GetUseGravity() const { return mUseGravity; }

		/** @copydoc Rigidbody::setVelocity */
		BS_SCRIPT_EXPORT(n:Velocity,pr:setter,hide)
		void SetVelocity(const Vector3& velocity);

		/** @copydoc Rigidbody::getVelocity */
		BS_SCRIPT_EXPORT(n:Velocity,pr:getter)
		Vector3 GetVelocity() const;

		/** @copydoc Rigidbody::setAngularVelocity */
		BS_SCRIPT_EXPORT(n:AngularVelocity,pr:setter,hide)
		void SetAngularVelocity(const Vector3& velocity);

		/** @copydoc Rigidbody::getAngularVelocity */
		BS_SCRIPT_EXPORT(n:AngularVelocity,pr:getter)
		Vector3 GetAngularVelocity() const;

		/** @copydoc Rigidbody::setDrag */
		BS_SCRIPT_EXPORT(n:Drag,pr:setter)
		void SetDrag(float drag);

		/** @copydoc Rigidbody::getDrag */
		BS_SCRIPT_EXPORT(n:Drag,pr:getter)
		float GetDrag() const { return mLinearDrag; }

		/** @copydoc Rigidbody::setAngularDrag */
		BS_SCRIPT_EXPORT(n:AngularDrag,pr:setter)
		void SetAngularDrag(float drag);

		/** @copydoc Rigidbody::getAngularDrag */
		BS_SCRIPT_EXPORT(n:AngularDrag,pr:getter)
		float GetAngularDrag() const { return mAngularDrag; }

		/** @copydoc Rigidbody::setInertiaTensor */
		BS_SCRIPT_EXPORT(n:InertiaTensor,pr:setter,hide)
		void SetInertiaTensor(const Vector3& tensor);

		/** @copydoc Rigidbody::getInertiaTensor */
		BS_SCRIPT_EXPORT(n:InertiaTensor,pr:getter)
		Vector3 GetInertiaTensor() const;

		/** @copydoc Rigidbody::setMaxAngularVelocity */
		BS_SCRIPT_EXPORT(n:MaxAngularVelocity,pr:setter,hide)
		void SetMaxAngularVelocity(float maxVelocity);

		/** @copydoc Rigidbody::getMaxAngularVelocity */
		BS_SCRIPT_EXPORT(n:MaxAngularVelocity,pr:getter)
		float GetMaxAngularVelocity() const { return mMaxAngularVelocity; }

		/** Determines the rigidbody's center of mass position. Only relevant if RigibodyFlag::AutoTensors is turned off. */
		BS_SCRIPT_EXPORT(n:CenterOfMassPosition,pr:setter,hide)
		void SetCenterOfMassPosition(const Vector3& position);

		/** @copydoc setCenterOfMassPosition() */
		BS_SCRIPT_EXPORT(n:CenterOfMassPosition,pr:getter)
		Vector3 GetCenterOfMassPosition() const;

		/** Determines the rigidbody's center of mass rotation. Only relevant if RigibodyFlag::AutoTensors is turned off. */
		BS_SCRIPT_EXPORT(n:CenterOfMassRotation,pr:setter,hide)
		void SetCenterOfMassRotation(const Quaternion& rotation);

		/** @copydoc setCenterOfMassRotation() */
		BS_SCRIPT_EXPORT(n:CenterOfMassRotation,pr:getter)
		Quaternion GetCenterOfMassRotation() const;

		/** @copydoc Rigidbody::setPositionSolverCount */
		BS_SCRIPT_EXPORT(n:PositionSolverCount,pr:setter,hide)
		void SetPositionSolverCount(UINT32 count);

		/** @copydoc Rigidbody::getPositionSolverCount */
		BS_SCRIPT_EXPORT(n:PositionSolverCount,pr:getter)
		UINT32 GetPositionSolverCount() const { return mPositionSolverCount; }

		/** @copydoc Rigidbody::setVelocitySolverCount */
		BS_SCRIPT_EXPORT(n:VelocitySolverCount,pr:setter,hide)
		void SetVelocitySolverCount(UINT32 count);

		/** @copydoc Rigidbody::getVelocitySolverCount */
		BS_SCRIPT_EXPORT(n:VelocitySolverCount,pr:getter)
		UINT32 GetVelocitySolverCount() const { return mVelocitySolverCount; }

		/** Sets a value that determines which (if any) collision events are reported. */
		BS_SCRIPT_EXPORT(n:CollisionReportMode,pr:setter)
		void SetCollisionReportMode(CollisionReportMode mode);

		/** Gets a value that determines which (if any) collision events are reported. */
		BS_SCRIPT_EXPORT(n:CollisionReportMode,pr:getter)
		CollisionReportMode GetCollisionReportMode() const { return mCollisionReportMode; }

		/** @copydoc Rigidbody::setFlags */
		BS_SCRIPT_EXPORT(n:Flags,pr:setter,hide)
		void SetFlags(RigidbodyFlag flags);

		/** @copydoc Rigidbody::getFlags */
		BS_SCRIPT_EXPORT(n:Flags,pr:getter)
		RigidbodyFlag GetFlags() const { return mFlags; }

		/** @copydoc Rigidbody::addForce */
		BS_SCRIPT_EXPORT(n:AddForce)
		void AddForce(const Vector3& force, ForceMode mode = ForceMode::Force);

		/** @copydoc Rigidbody::addTorque */
		BS_SCRIPT_EXPORT(n:AddTorque)
		void AddTorque(const Vector3& torque, ForceMode mode = ForceMode::Force);

		/** @copydoc Rigidbody::addForceAtPoint */
		BS_SCRIPT_EXPORT(n:AddForceAtPoint)
		void addForceAtPoint(const Vector3& force, const Vector3& position,
			PointForceMode mode = PointForceMode::Force);

		/** @copydoc Rigidbody::getVelocityAtPoint */
		BS_SCRIPT_EXPORT(n:GetVelocityAtPoint)
		Vector3 GetVelocityAtPoint(const Vector3& point) const;

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
		Rigidbody* _getInternal() const { return mInternal.Get(); }

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
		void UpdateColliders();

		/** Unregisters all child colliders from the Rigidbody. */
		void ClearColliders();

		/**
		 * Registers a new collider with the Rigidbody. This collider will then be used to calculate Rigidbody's geometry
		 * used for collisions, and optionally (depending on set flags) total mass, inertia tensors and center of mass.
		 */
		void AddCollider(const HCollider& collider);

		/** Unregisters the collider from the Rigidbody. */
		void RemoveCollider(const HCollider& collider);

		/** Checks if the rigidbody is nested under another rigidbody, and throws out a warning if so. */
		void CheckForNestedRigibody();

		/** Appends Component referenes for the colliders to the collision data. */
		void ProcessCollisionData(const CollisionDataRaw& raw, CollisionData& output);

		/** Destroys the internal rigidbody representation. */
		void DestroyInternal();

		/** Triggered when the internal rigidbody begins touching another object. */
		void TriggerOnCollisionBegin(const CollisionDataRaw& data);

		/** Triggered when the internal rigidbody continues touching another object. */
		void TriggerOnCollisionStay(const CollisionDataRaw& data);

		/** Triggered when the internal rigidbody ends touching another object. */
		void TriggerOnCollisionEnd(const CollisionDataRaw& data);

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		/** @copydoc Component::onInitialized() */
		void OnInitialized() override;

		/** @copydoc Component::onDestroyed() */
		void OnDestroyed() override;

		/** @copydoc Component::onDisabled() */
		void OnDisabled() override;

		/** @copydoc Component::onEnabled() */
		void OnEnabled() override;

		/** @copydoc Component::onTransformChanged() */
		void OnTransformChanged(TransformChangedFlags flags) override;

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
