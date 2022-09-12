//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsJoint.h"
#include "Scene/BsComponent.h"

namespace bs
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	Joint
	 *
	 * @note Wraps Joint as a Component.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Physics,n:Joint) CJoint : public Component
	{
	public:
		CJoint(const HSceneObject& parent, JOINT_DESC& desc);
		virtual ~CJoint() = default;

		/** @copydoc Joint::getBody */
		BS_SCRIPT_EXPORT(n:GetBody)
		HRigidbody GetBody(JointBody body) const;

		/** @copydoc Joint::setBody */
		BS_SCRIPT_EXPORT(n:SetBody)
		void SetBody(JointBody body, const HRigidbody& value);

		/** @copydoc Joint::getPosition */
		BS_SCRIPT_EXPORT(n:GetPosition)
		Vector3 GetPosition(JointBody body) const;

		/** @copydoc Joint::getRotation */
		BS_SCRIPT_EXPORT(n:GetRotation)
		Quaternion GetRotation(JointBody body) const;

		/** @copydoc Joint::setTransform */
		BS_SCRIPT_EXPORT(n:SetTransform)
		void SetTransform(JointBody body, const Vector3& position, const Quaternion& rotation);

		/** @copydoc Joint::getBreakForce */
		BS_SCRIPT_EXPORT(n:BreakForce,pr:getter)
		float GetBreakForce() const;

		/** @copydoc Joint::setBreakForce */
		BS_SCRIPT_EXPORT(n:BreakForce,pr:setter)
		void SetBreakForce(float force);

		/** @copydoc Joint::getBreakTorque */
		BS_SCRIPT_EXPORT(n:BreakTorque,pr:getter)
		float GetBreakTorque() const;

		/** @copydoc Joint::setBreakTorque */
		BS_SCRIPT_EXPORT(n:BreakTorque,pr:setter)
		void SetBreakTorque(float torque);

		/** @copydoc Joint::getEnableCollision */
		BS_SCRIPT_EXPORT(n:EnableCollision,pr:getter)
		bool GetEnableCollision() const;

		/** @copydoc Joint::setEnableCollision */
		BS_SCRIPT_EXPORT(n:EnableCollision,pr:setter)
		void SetEnableCollision(bool value);

		/** @copydoc Joint::onJointBreak */
		BS_SCRIPT_EXPORT(n:OnJointBreak)
		Event<void()> onJointBreak;

		/** @name Internal
		 *  @{
		 */

		/** Returns the Joint implementation wrapped by this component. */
		Joint* _getInternal() const { return mInternal.Get(); }

		/** @} */

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

	protected:
		friend class CRigidbody;
		using Component::destroyInternal;

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

		/** Checks can the provided rigidbody be used for initializing the joint. */
		bool IsBodyValid(const HRigidbody& body);

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
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

		CJoint(JOINT_DESC& desc); // Serialization only
	 };

	 /** @} */
}
