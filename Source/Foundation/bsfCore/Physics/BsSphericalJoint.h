//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsJoint.h"

namespace bs
{
	class PhysicsScene;
	/** @addtogroup Physics
	 *  @{
	 */

	struct SPHERICAL_JOINT_DESC;

	/** Flags that control options for the spherical joint */
	enum class BS_SCRIPT_EXPORT(m:Physics) SphericalJointFlag
	{
		Limit = 0x1 /**< Enables the cone range limit. */
	};

	/** 
	 * A spherical joint removes all translational degrees of freedom but allows all rotational degrees of freedom. 
	 * Essentially this ensures that the anchor points of the two bodies are always coincident. Bodies are allowed to
	 * rotate around the anchor points, and their rotatation can be limited by an elliptical cone.
	 */
	class BS_CORE_EXPORT SphericalJoint : public Joint
	{
	public:
		SphericalJoint(const SPHERICAL_JOINT_DESC& desc) { }
		virtual ~SphericalJoint() = default;

		/** @copydoc setLimit() */
		virtual LimitConeRange getLimit() const = 0;

		/** 
		 * Determines the limit of the joint. This clamps the rotation inside an eliptical angular cone. You must enable
		 * limit flag on the joint in order for this to be recognized. 
		 */
		virtual void setLimit(const LimitConeRange& limit) = 0;

		/** Enables or disables a flag that controls the joint's behaviour. */
		virtual void setFlag(SphericalJointFlag flag, bool enabled) = 0;

		/** Checks is the specified flag enabled. */
		virtual bool hasFlag(SphericalJointFlag flag) const = 0;

		/** 
		 * Creates a new spherical joint. 
		 * 
		 * @param[in]	scene		Scene to which to add the joint.
		 * @param[in]	desc		Settings describing the joint.
		 */
		static SPtr<SphericalJoint> create(PhysicsScene& scene, const SPHERICAL_JOINT_DESC& desc);
	};

	/** Structure used for initializing a new SphericalJoint. */
	struct SPHERICAL_JOINT_DESC : JOINT_DESC
	{
		LimitConeRange limit;
		SphericalJointFlag flag = (SphericalJointFlag)0;
	};

	/** @} */
}