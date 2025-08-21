//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsFixedJoint.h"
#include "Components/BsCJoint.h"
#include "Physics/BsJoint.h"

namespace b3d
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	class IFixedJointImplementation;

	/** Structure used for initializing a new FixedJoint. */
	struct FixedJointCreateInformation : JointCreateInformation
	{};

	/**
	 * @copydoc	FixedJoint
	 *
	 * @note	Wraps FixedJoint as a Component.
	 */
	class B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportName(FixedJoint)) CFixedJoint : public CJoint
	{
	public:
		CFixedJoint(const HSceneObject& parent);

		/** @name Internal
		 *  @{
		 */

		/** Returns the low level joint implementation. */
		IFixedJointImplementation& GetImplementation() const { return static_cast<IFixedJointImplementation&>(*mImplementation); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		SPtr<Joint> CreateInternal() override;
		void GetLocalTransform(JointBody body, Vector3& position, Quaternion& rotation) override;

		FixedJointCreateInformation mInformation;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CFixedJointRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;

	protected:
		CFixedJoint(); // Serialization only
	};

	/** Low-level interface for a joint used by the FixedJoint component. Should be implemented by the physics plugin to provide joint functionality. */
	class B3D_CORE_EXPORT IFixedJointImplementation : public IJointImplementation
	{ };

	/** @} */
} // namespace b3d
