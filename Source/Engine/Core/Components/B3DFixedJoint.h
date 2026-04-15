//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Components/B3DJoint.h"

namespace b3d
{
	/** @addtogroup Physics
	 *  @{
	 */

	class IFixedJointImplementation;

	/** Structure used for initializing a new FixedJoint. */
	struct FixedJointCreateInformation : JointCreateInformation
	{};

	/** Physics joint that will maintain a fixed distance and orientation between its two attached bodies. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) FixedJoint : public Joint
	{
	public:
		FixedJoint(const HSceneObject& parent);

		/** @name Internal
		 *  @{
		 */

		/** Returns the low level joint implementation. */
		IFixedJointImplementation& GetImplementation() const;

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		UPtr<IJointImplementation> CreateImplementation() override;
		void CalculateLocalBodyTransform(JointBody body, Vector3& position, Quaternion& rotation) override;

		FixedJointCreateInformation mInformation;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class FixedJointRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;

	protected:
		FixedJoint(); // Serialization only
	};

	/** @} */

	/** @addtogroup Physics-Internal
	 *  @{
	 */

	/** Low-level interface for a joint used by the FixedJoint component. Should be implemented by the physics plugin to provide joint functionality. */
	class B3D_EXPORT IFixedJointImplementation : public IJointImplementation
	{ };

	/** @} */
} // namespace b3d
