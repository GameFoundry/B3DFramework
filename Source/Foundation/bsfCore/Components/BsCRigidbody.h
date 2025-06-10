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
	class B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportName(Rigidbody)) CRigidbody : public Component
	{
	public:
		CRigidbody(const HSceneObject& parent);

		/** @copydoc Rigidbody::Move */
		B3D_SCRIPT_EXPORT(ExportName(Move))
		void Move(const Vector3& position);

		/** @copydoc Rigidbody::Rotate */
		B3D_SCRIPT_EXPORT(ExportName(Rotate))
		void Rotate(const Quaternion& rotation);

		/** @copydoc Rigidbody::SetMass */
		B3D_SCRIPT_EXPORT(ExportName(Mass), Property(Setter))
		void SetMass(float mass);

		/** @copydoc Rigidbody::GetMass */
		B3D_SCRIPT_EXPORT(ExportName(Mass), Property(Getter))
		float GetMass() const { return mMass; };

		/** @copydoc Rigidbody::SetIsKinematic */
		B3D_SCRIPT_EXPORT(ExportName(IsKinematic), Property(Setter))
		void SetIsKinematic(bool kinematic);

		/** @copydoc Rigidbody::GetIsKinematic */
		B3D_SCRIPT_EXPORT(ExportName(IsKinematic), Property(Getter))
		bool GetIsKinematic() const { return mIsKinematic; }

		/** @copydoc Rigidbody::IsSleeping */
		B3D_SCRIPT_EXPORT(ExportName(IsSleeping), Property(Getter))
		bool IsSleeping() const;

		/** @copydoc Rigidbody::Sleep */
		B3D_SCRIPT_EXPORT(ExportName(Sleep))
		void Sleep();

		/** @copydoc Rigidbody::WakeUp */
		B3D_SCRIPT_EXPORT(ExportName(WakeUp))
		void WakeUp();

		/** @copydoc Rigidbody::SetSleepThreshold */
		B3D_SCRIPT_EXPORT(ExportName(SleepThreshold), Property(Setter), UI(Hide))
		void SetSleepThreshold(float threshold);

		/** @copydoc Rigidbody::GetSleepThreshold */
		B3D_SCRIPT_EXPORT(ExportName(SleepThreshold), Property(Getter))
		float GetSleepThreshold() const { return mSleepThreshold; }

		/** @copydoc Rigidbody::SetUseGravity */
		B3D_SCRIPT_EXPORT(ExportName(UseGravity), Property(Setter))
		void SetUseGravity(bool gravity);

		/** @copydoc Rigidbody::GetUseGravity */
		B3D_SCRIPT_EXPORT(ExportName(UseGravity), Property(Getter))
		bool GetUseGravity() const { return mUseGravity; }

		/** @copydoc Rigidbody::SetVelocity */
		B3D_SCRIPT_EXPORT(ExportName(Velocity), Property(Setter), UI(Hide))
		void SetVelocity(const Vector3& velocity);

		/** @copydoc Rigidbody::GetVelocity */
		B3D_SCRIPT_EXPORT(ExportName(Velocity), Property(Getter))
		Vector3 GetVelocity() const;

		/** @copydoc Rigidbody::SetAngularVelocity */
		B3D_SCRIPT_EXPORT(ExportName(AngularVelocity), Property(Setter), UI(Hide))
		void SetAngularVelocity(const Vector3& velocity);

		/** @copydoc Rigidbody::GetAngularVelocity */
		B3D_SCRIPT_EXPORT(ExportName(AngularVelocity), Property(Getter))
		Vector3 GetAngularVelocity() const;

		/** @copydoc Rigidbody::SetDrag */
		B3D_SCRIPT_EXPORT(ExportName(Drag), Property(Setter))
		void SetDrag(float drag);

		/** @copydoc Rigidbody::GetDrag */
		B3D_SCRIPT_EXPORT(ExportName(Drag), Property(Getter))
		float GetDrag() const { return mLinearDrag; }

		/** @copydoc Rigidbody::SetAngularDrag */
		B3D_SCRIPT_EXPORT(ExportName(AngularDrag), Property(Setter))
		void SetAngularDrag(float drag);

		/** @copydoc Rigidbody::GetAngularDrag */
		B3D_SCRIPT_EXPORT(ExportName(AngularDrag), Property(Getter))
		float GetAngularDrag() const { return mAngularDrag; }

		/** @copydoc Rigidbody::SetInertiaTensor */
		B3D_SCRIPT_EXPORT(ExportName(InertiaTensor), Property(Setter), UI(Hide))
		void SetInertiaTensor(const Vector3& tensor);

		/** @copydoc Rigidbody::GetInertiaTensor */
		B3D_SCRIPT_EXPORT(ExportName(InertiaTensor), Property(Getter))
		Vector3 GetInertiaTensor() const;

		/** @copydoc Rigidbody::SetMaxAngularVelocity */
		B3D_SCRIPT_EXPORT(ExportName(MaxAngularVelocity), Property(Setter), UI(Hide))
		void SetMaxAngularVelocity(float maxVelocity);

		/** @copydoc Rigidbody::GetMaxAngularVelocity */
		B3D_SCRIPT_EXPORT(ExportName(MaxAngularVelocity), Property(Getter))
		float GetMaxAngularVelocity() const { return mMaxAngularVelocity; }

		/** Determines the rigidbody's center of mass position. Only relevant if RigibodyFlag::AutoTensors is turned off. */
		B3D_SCRIPT_EXPORT(ExportName(CenterOfMassPosition), Property(Setter), UI(Hide))
		void SetCenterOfMassPosition(const Vector3& position);

		/** @copydoc SetCenterOfMassPosition() */
		B3D_SCRIPT_EXPORT(ExportName(CenterOfMassPosition), Property(Getter))
		Vector3 GetCenterOfMassPosition() const;

		/** Determines the rigidbody's center of mass rotation. Only relevant if RigibodyFlag::AutoTensors is turned off. */
		B3D_SCRIPT_EXPORT(ExportName(CenterOfMassRotation), Property(Setter), UI(Hide))
		void SetCenterOfMassRotation(const Quaternion& rotation);

		/** @copydoc SetCenterOfMassRotation() */
		B3D_SCRIPT_EXPORT(ExportName(CenterOfMassRotation), Property(Getter))
		Quaternion GetCenterOfMassRotation() const;

		/** @copydoc Rigidbody::SetPositionSolverCount */
		B3D_SCRIPT_EXPORT(ExportName(PositionSolverCount), Property(Setter), UI(Hide))
		void SetPositionSolverCount(u32 count);

		/** @copydoc Rigidbody::GetPositionSolverCount */
		B3D_SCRIPT_EXPORT(ExportName(PositionSolverCount), Property(Getter))
		u32 GetPositionSolverCount() const { return mPositionSolverCount; }

		/** @copydoc Rigidbody::SetVelocitySolverCount */
		B3D_SCRIPT_EXPORT(ExportName(VelocitySolverCount), Property(Setter), UI(Hide))
		void SetVelocitySolverCount(u32 count);

		/** @copydoc Rigidbody::GetVelocitySolverCount */
		B3D_SCRIPT_EXPORT(ExportName(VelocitySolverCount), Property(Getter))
		u32 GetVelocitySolverCount() const { return mVelocitySolverCount; }

		/** Sets a value that determines which (if any) collision events are reported. */
		B3D_SCRIPT_EXPORT(ExportName(CollisionReportMode), Property(Setter))
		void SetCollisionReportMode(CollisionReportMode mode);

		/** Gets a value that determines which (if any) collision events are reported. */
		B3D_SCRIPT_EXPORT(ExportName(CollisionReportMode), Property(Getter))
		CollisionReportMode GetCollisionReportMode() const { return mCollisionReportMode; }

		/** @copydoc Rigidbody::SetFlags */
		B3D_SCRIPT_EXPORT(ExportName(Flags), Property(Setter), UI(Hide))
		void SetFlags(RigidbodyFlag flags);

		/** @copydoc Rigidbody::GetFlags */
		B3D_SCRIPT_EXPORT(ExportName(Flags), Property(Getter))
		RigidbodyFlag GetFlags() const { return mFlags; }

		/** @copydoc Rigidbody::AddForce */
		B3D_SCRIPT_EXPORT(ExportName(AddForce))
		void AddForce(const Vector3& force, ForceMode mode = ForceMode::Force);

		/** @copydoc Rigidbody::AddTorque */
		B3D_SCRIPT_EXPORT(ExportName(AddTorque))
		void AddTorque(const Vector3& torque, ForceMode mode = ForceMode::Force);

		/** @copydoc Rigidbody::AddForceAtPoint */
		B3D_SCRIPT_EXPORT(ExportName(AddForceAtPoint))
		void AddForceAtPoint(const Vector3& force, const Vector3& position, PointForceMode mode = PointForceMode::Force);

		/** @copydoc Rigidbody::GetVelocityAtPoint */
		B3D_SCRIPT_EXPORT(ExportName(GetVelocityAtPoint))
		Vector3 GetVelocityAtPoint(const Vector3& point) const;

		/** @copydoc Rigidbody::OnCollisionBegin */
		B3D_SCRIPT_EXPORT(ExportName(OnCollisionBegin))
		Event<void(const CollisionData&)> OnCollisionBegin;

		/** @copydoc Rigidbody::OnCollisionStay */
		B3D_SCRIPT_EXPORT(ExportName(OnCollisionStay))
		Event<void(const CollisionData&)> OnCollisionStay;

		/** @copydoc Rigidbody::OnCollisionEnd */
		B3D_SCRIPT_EXPORT(ExportName(OnCollisionEnd))
		Event<void(const CollisionData&)> OnCollisionEnd;

		/** @name Internal
		 *  @{
		 */

		/** Returns the Rigidbody implementation wrapped by this component. */
		Rigidbody* GetInternalInternal() const { return mInternal.get(); }

		/** Sets that joint that this rigidbody is attached to. Allows the rigidbody to notify the joint when it moves. */
		void SetJointInternal(const HJoint& joint) { mParentJoint = joint; }

		/** @copydoc Rigidbody::updateMassDistribution */
		void UpdateMassDistribution();

		/** @} */
	protected:
		friend class CCollider;

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

		void OnBeginPlay() override;
		void OnDestroyed() override;
		void OnDisabled() override;
		void OnEnabled() override;
		void OnTransformChanged(TransformChangedFlags flags) override;

		SPtr<Rigidbody> mInternal;
		Vector<HCollider> mChildren;
		HJoint mParentJoint;

		u32 mPositionSolverCount = 4;
		u32 mVelocitySolverCount = 1;
		RigidbodyFlag mFlags = (RigidbodyFlag)((u32)RigidbodyFlag::AutoTensors | (u32)RigidbodyFlag::AutoMass);
		CollisionReportMode mCollisionReportMode = CollisionReportMode::None;
		Vector3 mCMassPosition = Vector3::kZero;
		Quaternion mCMassRotation = Quaternion::kIdentity;
		Vector3 mInertiaTensor = Vector3::kZero;
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
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;

	protected:
		CRigidbody(); // Serialization only
	};

	/** @} */
} // namespace bs
