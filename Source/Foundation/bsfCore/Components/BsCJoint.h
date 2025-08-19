//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsJoint.h"
#include "Scene/BsComponent.h"

namespace b3d
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	Joint
	 *
	 * @note Wraps Joint as a Component.
	 */
	class B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportName(Joint)) CJoint : public Component
	{
	public:
		CJoint(const HSceneObject& parent, JOINT_DESC& desc);
		virtual ~CJoint() = default;

		/** @copydoc Joint::GetBody */
		B3D_SCRIPT_EXPORT(ExportName(GetBody))
		HRigidbody GetBody(JointBody body) const;

		/** @copydoc Joint::SetBody */
		B3D_SCRIPT_EXPORT(ExportName(SetBody))
		void SetBody(JointBody body, const HRigidbody& value);

		/** @copydoc Joint::GetPosition */
		B3D_SCRIPT_EXPORT(ExportName(GetPosition))
		Vector3 GetPosition(JointBody body) const;

		/** @copydoc Joint::GetRotation */
		B3D_SCRIPT_EXPORT(ExportName(GetRotation))
		Quaternion GetRotation(JointBody body) const;

		/** @copydoc Joint::SetTransform */
		B3D_SCRIPT_EXPORT(ExportName(SetTransform))
		void SetTransform(JointBody body, const Vector3& position, const Quaternion& rotation);

		/** @copydoc Joint::GetBreakForce */
		B3D_SCRIPT_EXPORT(ExportName(BreakForce), Property(Getter))
		float GetBreakForce() const;

		/** @copydoc Joint::SetBreakForce */
		B3D_SCRIPT_EXPORT(ExportName(BreakForce), Property(Setter))
		void SetBreakForce(float force);

		/** @copydoc Joint::GetBreakTorque */
		B3D_SCRIPT_EXPORT(ExportName(BreakTorque), Property(Getter))
		float GetBreakTorque() const;

		/** @copydoc Joint::SetBreakTorque */
		B3D_SCRIPT_EXPORT(ExportName(BreakTorque), Property(Setter))
		void SetBreakTorque(float torque);

		/** @copydoc Joint::GetEnableCollision */
		B3D_SCRIPT_EXPORT(ExportName(EnableCollision), Property(Getter))
		bool GetEnableCollision() const;

		/** @copydoc Joint::SetEnableCollision */
		B3D_SCRIPT_EXPORT(ExportName(EnableCollision), Property(Setter))
		void SetEnableCollision(bool value);

		/** @copydoc Joint::OnJointBreak */
		B3D_SCRIPT_EXPORT(ExportName(OnJointBreak))
		Event<void()> OnJointBreak;

		/** @name Internal
		 *  @{
		 */

		/** Returns the Joint implementation wrapped by this component. */
		Joint* GetInternalInternal() const { return mInternal.get(); }

		/** @} */

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

	protected:
		friend class CRigidbody;

		/** Creates the internal representation of the Joint for use by the component. */
		virtual SPtr<Joint> CreateInternal() = 0;

		/** Creates the internal representation of the Joint and restores the values saved by the Component. */
		virtual void RestoreInternal();

		/** Calculates the local position/rotation that needs to be applied to the particular joint body. */
		virtual void GetLocalTransform(JointBody body, Vector3& position, Quaternion& rotation);

		/** Destroys the internal joint representation. */
		void DestroyInternal();

		/** Notifies the joint that one of the attached rigidbodies moved and that its transform needs updating. */
		void NotifyRigidbodyMoved(const HRigidbody& body);

		/** Updates the local transform for the specified body attached to the joint. */
		void UpdateTransform(JointBody body);

		/** Triggered when the joint constraint gets broken. */
		void TriggerOnJointBroken();

		SPtr<Joint> mInternal;

		HRigidbody mBodies[2];
		Vector3 mPositions[2];
		Quaternion mRotations[2];

	private:
		JOINT_DESC& mDesc;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CJointRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;

		CJoint(JOINT_DESC& desc); // Serialization only
	};

	/** @} */
} // namespace b3d
