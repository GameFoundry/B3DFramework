//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Scene/B3DComponent.h"
#include "Physics/B3DPhysicsCommon.h"
#include "Math/B3DVector3.h"
#include "Math/B3DQuaternion.h"

namespace b3d
{
	class PhysicsScene;
	class IRigidbodyImplementation;

	/** @addtogroup Components
	 *  @{
	 */

	/** Type of force or torque that can be applied to a rigidbody. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) ForceMode
	{
		Force, /**< Value applied is a force. */
		Impulse, /**< Value applied is an impulse (a direct change in its linear or angular momentum). */
		Velocity, /**< Value applied is velocity. */
		Acceleration /**< Value applied is accelearation. */
	};

	/** Type of force that can be applied to a rigidbody at an arbitrary point. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) PointForceMode
	{
		Force, /**< Value applied is a force. */
		Impulse, /**< Value applied is an impulse (a direct change in its linear or angular momentum). */
	};

	/** Flags that control options of a Rigidbody object. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) RigidbodyFlag
	{
		/** No options. */
		None = 0x00,
		/** Automatically calculate center of mass transform and inertia tensors from child shapes (colliders). */
		AutoTensors = 0x01,
		/** Calculate mass distribution from child shapes (colliders). Only relevant when auto-tensors is on. */
		AutoMass = 0x02,
		/**
		 * Enables continous collision detection. This can prevent fast moving bodies from tunneling through each other.
		 * This must also be enabled globally in Physics otherwise the flag will be ignored.
		 */
		CCD = 0x04
	};

	using RigidbodyFlags = Flags<RigidbodyFlag>;
	B3D_FLAGS_OPERATORS(RigidbodyFlag)

	/**
	 * Rigidbody is a dynamic physics object that can be moved using forces (or directly). It will interact with other
	 * static and dynamic physics objects in the scene accordingly (it will push other non-kinematic rigidbodies,
	 * and collide with static objects).
	 *
	 * The shape and mass of a rigidbody is governed by its colliders. At least one collider must be attached to the collider.
	 * To attach a collider, place it on the same scene object as the rigidbody, or a child scene object.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) Rigidbody : public Component
	{
	public:
		Rigidbody(const HSceneObject& parent);

		/**
		 * Moves the rigidbody to a specific position. This method will ensure physically correct movement, meaning the body
		 * will collide with other objects along the way.
		 */
		B3D_SCRIPT_EXPORT(ExportName(Move))
		void Move(const Vector3& position);

		/**
		 * Rotates the rigidbody. This method will ensure physically correct rotation, meaning the body will collide with
		 * other objects along the way.
		 */
		B3D_SCRIPT_EXPORT(ExportName(Rotate))
		void Rotate(const Quaternion& rotation);

		/**
		 * Determines the mass of the object and all of its collider shapes. Only relevant if RigidbodyFlag::AutoMass or
		 * RigidbodyFlag::AutoTensors is turned off. Value of zero means the object is immovable (but can be rotated).
		 */
		B3D_SCRIPT_EXPORT(ExportName(Mass), Property(Setter))
		void SetMass(float mass);

		/** @copydoc GetMass */
		B3D_SCRIPT_EXPORT(ExportName(Mass), Property(Getter))
		float GetMass() const { return mMass; }

		/**
		 * Determines if the body is kinematic. Kinematic body will not move in response to external forces (for example
		 * gravity, or another object pushing it), essentially behaving like collider. Unlike a collider though, you can
		 * still move the object and have other dynamic objects respond correctly (meaning it will push other objects).
		 */
		B3D_SCRIPT_EXPORT(ExportName(IsKinematic), Property(Setter))
		void SetIsKinematic(bool kinematic);

		/** @copydoc GetIsKinematic */
		B3D_SCRIPT_EXPORT(ExportName(IsKinematic), Property(Getter))
		bool GetIsKinematic() const { return mIsKinematic; }

		/**
		 * Checks if the body is sleeping. Objects that aren't moved/rotated for a while are put to sleep to reduce load
		 * on the physics system.
		 */
		B3D_SCRIPT_EXPORT(ExportName(IsSleeping), Property(Getter))
		bool IsSleeping() const;

		/** Forces the object to sleep. Useful if you know the object will not move in any significant way for a while. */
		B3D_SCRIPT_EXPORT()
		void Sleep();

		/**
		 * Wakes an object up. Useful if you modified properties of this object, and potentially surrounding objects which
		 * might result in the object being moved by physics (although the physics system will automatically wake the
		 * object up for majority of such cases).
		 */
		B3D_SCRIPT_EXPORT()
		void WakeUp();

		/** Determines a threshold of force and torque under which the object will be considered to be put to sleep. */
		B3D_SCRIPT_EXPORT(ExportName(SleepThreshold), Property(Setter), UI(Hide))
		void SetSleepThreshold(float threshold);

		/** @copydoc GetSleepThreshold */
		B3D_SCRIPT_EXPORT(ExportName(SleepThreshold), Property(Getter))
		float GetSleepThreshold() const { return mSleepThreshold; }

		/** Determines whether or not the rigidbody will have the global gravity force applied to it. */
		B3D_SCRIPT_EXPORT(ExportName(UseGravity), Property(Setter))
		void SetUseGravity(bool gravity);

		/** @copydoc GetUseGravity */
		B3D_SCRIPT_EXPORT(ExportName(UseGravity), Property(Getter))
		bool GetUseGravity() const { return mUseGravity; }

		/** Determines the linear velocity of the body. */
		B3D_SCRIPT_EXPORT(ExportName(Velocity), Property(Setter), UI(Hide))
		void SetVelocity(const Vector3& velocity);

		/** @copydoc GetVelocity */
		B3D_SCRIPT_EXPORT(ExportName(Velocity), Property(Getter))
		Vector3 GetVelocity() const;

		/** Determines the angular velocity of the body. */
		B3D_SCRIPT_EXPORT(ExportName(AngularVelocity), Property(Setter), UI(Hide))
		void SetAngularVelocity(const Vector3& velocity);

		/** @copydoc GetAngularVelocity */
		B3D_SCRIPT_EXPORT(ExportName(AngularVelocity), Property(Getter))
		Vector3 GetAngularVelocity() const;

		/** Determines the linear drag of the body. Higher drag values means the object resists linear movement more. */
		B3D_SCRIPT_EXPORT(ExportName(Drag), Property(Setter))
		void SetDrag(float drag);

		/** @copydoc GetDrag */
		B3D_SCRIPT_EXPORT(ExportName(Drag), Property(Getter))
		float GetDrag() const { return mLinearDrag; }

		/** Determines the angular drag of the body. Higher drag values means the object resists angular movement more. */
		B3D_SCRIPT_EXPORT(ExportName(AngularDrag), Property(Setter))
		void SetAngularDrag(float drag);

		/** @copydoc GetAngularDrag */
		B3D_SCRIPT_EXPORT(ExportName(AngularDrag), Property(Getter))
		float GetAngularDrag() const { return mAngularDrag; }

		/**
		 * Determines the inertia tensor in local mass space. Inertia tensor determines how difficult is to rotate the
		 * object. Values of zero in the inertia tensor mean the object will be unable to rotate around a specific axis.
		 * Only relevant if RigidbodyFlag::AutoTensors is turned off.
		 */
		B3D_SCRIPT_EXPORT(ExportName(InertiaTensor), Property(Setter), UI(Hide))
		void SetInertiaTensor(const Vector3& tensor);

		/** @copydoc GetInertiaTensor */
		B3D_SCRIPT_EXPORT(ExportName(InertiaTensor), Property(Getter))
		Vector3 GetInertiaTensor() const;

		/** Determines the maximum angular velocity of the rigidbody. Velocity will be clamped to this value. */
		B3D_SCRIPT_EXPORT(ExportName(MaxAngularVelocity), Property(Setter), UI(Hide))
		void SetMaxAngularVelocity(float velocity);

		/** @copydoc GetMaxAngularVelocity */
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

		/**
		 * Determines the number of iterations to use when solving for position. Higher values can improve precision and
		 * numerical stability of the simulation.
		 */
		B3D_SCRIPT_EXPORT(ExportName(PositionSolverCount), Property(Setter), UI(Hide))
		void SetPositionSolverCount(u32 count);

		/** @copydoc GetPositionSolverCount */
		B3D_SCRIPT_EXPORT(ExportName(PositionSolverCount), Property(Getter))
		u32 GetPositionSolverCount() const { return mPositionSolverCount; }

		/**
		 * Determines the number of iterations to use when solving for velocity. Higher values can improve precision and
		 * numerical stability of the simulation.
		 */
		B3D_SCRIPT_EXPORT(ExportName(VelocitySolverCount), Property(Setter), UI(Hide))
		void SetVelocitySolverCount(u32 count);

		/** @copydoc GetVelocitySolverCount */
		B3D_SCRIPT_EXPORT(ExportName(VelocitySolverCount), Property(Getter))
		u32 GetVelocitySolverCount() const { return mVelocitySolverCount; }

		/** Sets a value that determines which (if any) collision events are reported. */
		B3D_SCRIPT_EXPORT(ExportName(CollisionReportMode), Property(Setter))
		void SetCollisionReportMode(CollisionReportMode mode);

		/** Gets a value that determines which (if any) collision events are reported. */
		B3D_SCRIPT_EXPORT(ExportName(CollisionReportMode), Property(Getter))
		CollisionReportMode GetCollisionReportMode() const { return mCollisionReportMode; }

		/** Flags that control the behaviour of the rigidbody. */
		B3D_SCRIPT_EXPORT(ExportName(Flags), Property(Setter), UI(Hide))
		void SetFlags(RigidbodyFlags flags);

		/** @copydoc GetFlags */
		B3D_SCRIPT_EXPORT(ExportName(Flags), Property(Getter))
		RigidbodyFlags GetFlags() const { return mFlags; }

		/**
		 * Applies a force to the center of the mass of the rigidbody. This will produce linear momentum.
		 *
		 * @param	force	Force to apply.
		 * @param	mode	Determines what is the type of @p force.
		 */
		B3D_SCRIPT_EXPORT()
		void AddForce(const Vector3& force, ForceMode mode = ForceMode::Force);

		/**
		 * Applies a torque to the rigidbody. This will produce angular momentum.
		 *
		 * @param	torque	Torque to apply.
		 * @param	mode	Determines what is the type of @p torque.
		 */
		B3D_SCRIPT_EXPORT()
		void AddTorque(const Vector3& torque, ForceMode mode = ForceMode::Force);

		/**
		 * Applies a force to a specific point on the rigidbody. This will in most cases produce both linear and angular
		 * momentum.
		 *
		 * @param	force		Force to apply.
		 * @param	position	World position to apply the force at.
		 * @param	mode		Determines what is the type of @p force.
		 */
		B3D_SCRIPT_EXPORT()
		void AddForceAtPoint(const Vector3& force, const Vector3& position, PointForceMode mode = PointForceMode::Force);

		/**
		 * Returns the total (linear + angular) velocity at a specific point.
		 *
		 * @param	point	Point in world space.
		 * @return			Total velocity of the point.
		 */
		B3D_SCRIPT_EXPORT()
		Vector3 GetVelocityAtPoint(const Vector3& point) const;

		/** Triggered when one of the colliders owned by the rigidbody starts colliding with another object. */
		B3D_SCRIPT_EXPORT()
		Event<void(const CollisionData&)> OnCollisionBegin;

		/** Triggered when a previously colliding collider stays in collision. Triggered once per frame. */
		B3D_SCRIPT_EXPORT()
		Event<void(const CollisionData&)> OnCollisionStay;

		/** Triggered when one of the colliders owned by the rigidbody stops colliding with another object. */
		B3D_SCRIPT_EXPORT()
		Event<void(const CollisionData&)> OnCollisionEnd;

		/** @name Internal
		 *  @{
		 */

		/** Returns the low level rigidbody implementation. */
		IRigidbodyImplementation& GetImplementation() const { return *mImplementation; }

		/** Sets that joint that this rigidbody is attached to. Allows the rigidbody to notify the joint when it moves. */
		void SetParentJoint(const HJoint& joint) { mParentJoint = joint; }

		/**
		 * Recalculates rigidbody's mass, inertia tensors and center of mass depending on the currently set child colliders.
		 * This should be called whenever relevant child collider properties change (like mass or shape).
		 *
		 * If automatic tensor calculation is turned off then this will do nothing. If automatic mass calculation is turned
		 * off then this will use the mass set directly on the body using SetMass().
		 */
		void UpdateMassDistribution();

		/** @} */
	protected:
		friend class Collider;

		/**
		 * Searches child scene objects for Collider components and attaches them to the rigidbody. Make sure to call
		 * ClearColliders() if you need to clear old colliders first.
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

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		void OnCreated() override;
		void OnDestroyed() override;
		void OnDisabled() override;
		void OnEnabled() override;
		void OnTransformChanged(TransformChangedFlags flags) override;

		UPtr<IRigidbodyImplementation> mImplementation;
		Vector<HCollider> mChildColliders;
		HJoint mParentJoint;

		u32 mPositionSolverCount = 4;
		u32 mVelocitySolverCount = 1;
		RigidbodyFlags mFlags = RigidbodyFlag::AutoTensors | RigidbodyFlag::AutoMass;
		CollisionReportMode mCollisionReportMode = CollisionReportMode::None;
		Vector3 mCenterOfMassPosition = Vector3::kZero;
		Quaternion mCenterOfMassRotation = Quaternion::kIdentity;
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
		friend class RigidbodyRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;

	protected:
		Rigidbody(); // Serialization only
	};

	/** Low-level interface for a rigidbody used by the Rigidbody component. Should be implemented by the physics plugin to provide rigidbody functionality. */
	class B3D_EXPORT IRigidbodyImplementation
	{
	public:
		virtual ~IRigidbodyImplementation() = default;

		/** @copydoc Rigidbody::Move */
		virtual void Move(const Vector3& position) = 0;

		/** @copydoc Rigidbody::Rotate */
		virtual void Rotate(const Quaternion& rotation) = 0;

		/** @copydoc Rigidbody::SetMass */
		virtual void SetMass(float mass) = 0;

		/** @copydoc Rigidbody::SetIsKinematic */
		virtual void SetIsKinematic(bool kinematic) = 0;

		/** @copydoc Rigidbody::IsSleeping */
		virtual bool IsSleeping() const = 0;

		/** @copydoc Rigidbody::Sleep */
		virtual void Sleep() = 0;

		/** @copydoc Rigidbody::WakeUp */
		virtual void WakeUp() = 0;

		/** @copydoc Rigidbody::SetSleepThreshold */
		virtual void SetSleepThreshold(float threshold) = 0;

		/** @copydoc Rigidbody::SetUseGravity */
		virtual void SetUseGravity(bool gravity) = 0;

		/** @copydoc Rigidbody::SetVelocity */
		virtual void SetVelocity(const Vector3& velocity) = 0;

		/** @copydoc Rigidbody::GetVelocity */
		virtual Vector3 GetVelocity() const = 0;

		/** @copydoc Rigidbody::SetAngularVelocity */
		virtual void SetAngularVelocity(const Vector3& velocity) = 0;

		/** @copydoc Rigidbody::GetAngularVelocity */
		virtual Vector3 GetAngularVelocity() const = 0;

		/** @copydoc Rigidbody::SetDrag */
		virtual void SetDrag(float drag) = 0;

		/** @copydoc Rigidbody::SetAngularDrag */
		virtual void SetAngularDrag(float drag) = 0;

		/** @copydoc Rigidbody::SetInertiaTensor */
		virtual void SetInertiaTensor(const Vector3& tensor) = 0;

		/** @copydoc Rigidbody::GetInertiaTensor */
		virtual Vector3 GetInertiaTensor() const = 0;

		/** @copydoc Rigidbody::SetMaxAngularVelocity */
		virtual void SetMaxAngularVelocity(float velocity) = 0;

		/** @copydoc Rigidbody::SetFlags */
		virtual void SetFlags(RigidbodyFlags flags) = 0;

		/** @copydoc Rigidbody::AddForce */
		virtual void AddForce(const Vector3& force, ForceMode mode = ForceMode::Force) = 0;

		/** @copydoc Rigidbody::AddTorque */
		virtual void AddTorque(const Vector3& torque, ForceMode mode = ForceMode::Force) = 0;

		/** @copydoc Rigidbody::AddForceAtPoint */
		virtual void AddForceAtPoint(const Vector3& force, const Vector3& position, PointForceMode mode = PointForceMode::Force) = 0;

		/** @copydoc Rigidbody::GetVelocityAtPoint */
		virtual Vector3 GetVelocityAtPoint(const Vector3& point) const = 0;

		/** @copydoc Rigidbody::UpdateMassDistribution */
		virtual void UpdateMassDistribution(bool autoMassEnabled) = 0;

		/**
		 * Sets the transform of the low-level physics rigidbody object. Unlike Move() and Rotate() this will not transform the
		 * body in a physically correct manner, but will instead "teleport" it immediately to the specified position and rotation.
		 */
		virtual void SetTransform(const Vector3& position, const Quaternion& rotation) = 0;

		/**
		 * Returns the transform that is currently assigned to the low-level physics rigidbody object. This may be the transform
		 * you explicitly set via SetTransform(), or a value that has been calculated by physics simulation for kinematic rigidbodies.
		 */
		virtual void GetTransform(Vector3& outPosition, Quaternion& outRotation) = 0;

		/**
		 * Sets the rigidbody's center of mass transform. Only relevant if RigibodyFlag::AutoTensors is turned off.
		 *
		 * @param	position	Position of the center of mass.
		 * @param	rotation	Rotation that determines orientation of the inertia tensor (rotation of the center of mass frame).
		 */
		virtual void SetCenterOfMass(const Vector3& position, const Quaternion& rotation) = 0;

		/**
		 * Gets the rigidbody's center of mass transform.
		 *
		 * @param	outPosition	Position of the center of mass.
		 * @param	outRotation	Rotation that determines orientation of the inertia tensor (rotation of the center of mass frame).
		 */
		virtual void GetCenterOfMass(Vector3& outPosition, Quaternion& outRotation) = 0;

		/**
		 * Determines the number of iterations to use when solving for position and velocity. Higher values can improve precision and
		 * numerical stability of the simulation.
		 */
		virtual void SetSolverIterationCounts(u32 positionCount, u32 velocityCount) = 0;

		/** Adds the rigidbody to the physics scene. */
		virtual void AddToScene(PhysicsScene& scene) = 0;

		/** Removes the rigidbody from the currently assigned physics scene. */
		virtual void RemoveFromScene() = 0;

		/** Assigns a new child shape to the rigidbody. */
		virtual void AttachShape(const SPtr<ColliderShape>& shape) = 0;

		/** Removes a shape that was previously attached to the rigidbody. */
		virtual void DetachShape(const SPtr<ColliderShape>& shape) = 0;
	};

	/** @} */
} // namespace b3d
