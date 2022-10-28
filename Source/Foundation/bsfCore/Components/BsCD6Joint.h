//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsD6Joint.h"
#include "Components/BsCJoint.h"

namespace bs
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	D6Joint
	 *
	 * @note	Wraps D6Joint as a Component.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportName(D6Joint)) CD6Joint : public CJoint
	{
	public:
		CD6Joint(const HSceneObject& parent);

		/** @copydoc D6Joint::GetMotion */
		BS_SCRIPT_EXPORT(ExportName(GetMotion))
		D6JointMotion GetMotion(D6JointAxis axis) const;

		/** @copydoc D6Joint::SetMotion */
		BS_SCRIPT_EXPORT(ExportName(SetMotion))
		void SetMotion(D6JointAxis axis, D6JointMotion motion);

		/** @copydoc D6Joint::GetTwist */
		BS_SCRIPT_EXPORT(ExportName(Twist), Property(Getter))
		Radian GetTwist() const;

		/** @copydoc D6Joint::GetSwingY */
		BS_SCRIPT_EXPORT(ExportName(SwingY), Property(Getter))
		Radian GetSwingY() const;

		/** @copydoc D6Joint::GetSwingZ */
		BS_SCRIPT_EXPORT(ExportName(SwingZ), Property(Getter))
		Radian GetSwingZ() const;

		/** @copydoc D6Joint::GetLimitLinear */
		BS_SCRIPT_EXPORT(ExportName(LimitLinear), Property(Getter))
		LimitLinear GetLimitLinear() const;

		/** @copydoc D6Joint::SetLimitLinear */
		BS_SCRIPT_EXPORT(ExportName(LimitLinear), Property(Setter))
		void SetLimitLinear(const LimitLinear& limit);

		/** @copydoc D6Joint::GetLimitTwist */
		BS_SCRIPT_EXPORT(ExportName(LimitTwist), Property(Getter))
		LimitAngularRange GetLimitTwist() const;

		/** @copydoc D6Joint::SetLimitTwist */
		BS_SCRIPT_EXPORT(ExportName(LimitTwist), Property(Setter))
		void SetLimitTwist(const LimitAngularRange& limit);

		/** @copydoc D6Joint::GetLimitSwing */
		BS_SCRIPT_EXPORT(ExportName(LimitSwing), Property(Getter))
		LimitConeRange GetLimitSwing() const;

		/** @copydoc D6Joint::SetLimitSwing */
		BS_SCRIPT_EXPORT(ExportName(LimitSwing), Property(Setter))
		void SetLimitSwing(const LimitConeRange& limit);

		/** @copydoc D6Joint::GetDrive */
		BS_SCRIPT_EXPORT(ExportName(GetDrive))
		D6JointDrive GetDrive(D6JointDriveType type) const;

		/** @copydoc D6Joint::SetDrive */
		BS_SCRIPT_EXPORT(ExportName(SetDrive))
		void SetDrive(D6JointDriveType type, const D6JointDrive& drive);

		/** @copydoc D6Joint::GetDrivePosition */
		BS_SCRIPT_EXPORT(ExportName(DrivePosition), Property(Getter))
		Vector3 GetDrivePosition() const;

		/** @copydoc D6Joint::GetDriveRotation */
		BS_SCRIPT_EXPORT(ExportName(DriveRotation), Property(Getter))
		Quaternion GetDriveRotation() const;

		/** @copydoc D6Joint::SetDriveTransform */
		BS_SCRIPT_EXPORT(ExportName(SetDriveTransform))
		void SetDriveTransform(const Vector3& position, const Quaternion& rotation);

		/** @copydoc D6Joint::GetDriveLinearVelocity */
		BS_SCRIPT_EXPORT(ExportName(DriveLinearVelocity), Property(Getter))
		Vector3 GetDriveLinearVelocity() const;

		/** @copydoc D6Joint::GetDriveAngularVelocity */
		BS_SCRIPT_EXPORT(ExportName(DriveAngularVelocity), Property(Getter))
		Vector3 GetDriveAngularVelocity() const;

		/** @copydoc D6Joint::SetDriveVelocity */
		BS_SCRIPT_EXPORT(ExportName(SetDriveVelocity))
		void SetDriveVelocity(const Vector3& linear, const Vector3& angular);

		/** @name Internal
		 *  @{
		 */

		/**	Returns the D6 joint that this component wraps. */
		D6Joint* GetInternalInternal() const { return static_cast<D6Joint*>(mInternal.get()); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		SPtr<Joint> CreateInternal() override;

		D6_JOINT_DESC mDesc;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CD6JointRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const;

	protected:
		CD6Joint(); // Serialization only
	};

	/** @} */
} // namespace bs
