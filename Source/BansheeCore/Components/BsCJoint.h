//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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
		virtual ~CJoint() {}

		/** @copydoc Joint::getBody */
		BS_SCRIPT_EXPORT(n:GetBody)
		HRigidbody getBody(JointBody body) const;

		/** @copydoc Joint::setBody */
		BS_SCRIPT_EXPORT(n:SetBody)
		void setBody(JointBody body, const HRigidbody& value);

		/** @copydoc Joint::getPosition */
		BS_SCRIPT_EXPORT(n:GetPosition)
		Vector3 getPosition(JointBody body) const;

		/** @copydoc Joint::getRotation */
		BS_SCRIPT_EXPORT(n:GetRotation)
		Quaternion getRotation(JointBody body) const;

		/** @copydoc Joint::setTransform */
		BS_SCRIPT_EXPORT(n:SetTransform)
		void setTransform(JointBody body, const Vector3& position, const Quaternion& rotation);

		/** @copydoc Joint::getBreakForce */
		BS_SCRIPT_EXPORT(n:BreakForce,pr:getter)
		float getBreakForce() const;

		/** @copydoc Joint::setBreakForce */
		BS_SCRIPT_EXPORT(n:BreakForce,pr:setter)
		void setBreakForce(float force);

		/** @copydoc Joint::getBreakTorque */
		BS_SCRIPT_EXPORT(n:BreakTorque,pr:getter)
		float getBreakTorque() const;

		/** @copydoc Joint::setBreakTorque */
		BS_SCRIPT_EXPORT(n:BreakTorque,pr:setter)
		void setBreakTorque(float torque);

		/** @copydoc Joint::getEnableCollision */
		BS_SCRIPT_EXPORT(n:EnableCollision,pr:getter)
		bool getEnableCollision() const;

		/** @copydoc Joint::setEnableCollision */
		BS_SCRIPT_EXPORT(n:EnableCollision,pr:setter)
		void setEnableCollision(bool value);

		/** @copydoc Joint::onJointBreak */
		BS_SCRIPT_EXPORT(n:OnJointBreak)
		Event<void()> onJointBreak;

		/** @name Internal
		 *  @{
		 */

		/** Returns the Joint implementation wrapped by this component. */
		Joint* _getInternal() const { return mInternal.get(); }

		/** @} */

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

	protected:
		friend class CRigidbody;
		using Component::destroyInternal;

		/** Creates the internal representation of the Joint for use by the component. */
		virtual SPtr<Joint> createInternal() = 0;

		/** Creates the internal representation of the Joint and restores the values saved by the Component. */
		virtual void restoreInternal();

		/** Calculates the local position/rotation that needs to be applied to the particular joint body. */
		virtual void getLocalTransform(JointBody body, Vector3& position, Quaternion& rotation);

		/** Destroys the internal joint representation. */
		void destroyInternal();

		/** Notifies the joint that one of the attached rigidbodies moved and that its transform needs updating. */
		void notifyRigidbodyMoved(const HRigidbody& body);

		/** Checks can the provided rigidbody be used for initializing the joint. */
		bool isBodyValid(const HRigidbody& body);

		/** Updates the local transform for the specified body attached to the joint. */
		void updateTransform(JointBody body);

		/** Triggered when the joint constraint gets broken. */
		void triggerOnJointBroken();

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