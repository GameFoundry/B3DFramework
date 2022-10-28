//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsSphericalJoint.h"
#include "Components/BsCJoint.h"

namespace bs
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	SphericalJoint
	 *
	 * @note	Wraps SphericalJoint as a Component.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportName(SphericalJoint)) CSphericalJoint : public CJoint
	{
	public:
		CSphericalJoint(const HSceneObject& parent);

		/** @copydoc SphericalJoint::GetLimit */
		BS_SCRIPT_EXPORT(ExportName(Limit), Property(Getter))
		LimitConeRange GetLimit() const;

		/** @copydoc SphericalJoint::SetLimit */
		BS_SCRIPT_EXPORT(ExportName(Limit), Property(Setter))
		void SetLimit(const LimitConeRange& limit);

		/** @copydoc SphericalJoint::SetFlag */
		BS_SCRIPT_EXPORT(ExportName(SetFlag))
		void SetFlag(SphericalJointFlag flag, bool enabled);

		/** @copydoc SphericalJoint::HasFlag */
		BS_SCRIPT_EXPORT(ExportName(HasFlag))
		bool HasFlag(SphericalJointFlag flag) const;

		/** @name Internal
		 *  @{
		 */

		/**	Returns the spherical joint that this component wraps. */
		SphericalJoint* GetInternalInternal() const { return static_cast<SphericalJoint*>(mInternal.get()); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		SPtr<Joint> CreateInternal() override;

		SPHERICAL_JOINT_DESC mDesc;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CSphericalJointRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const;

	protected:
		CSphericalJoint(); // Serialization only
	};

	/** @} */
} // namespace bs
