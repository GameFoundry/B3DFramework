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

	struct DISTANCE_JOINT_DESC;

	/** Controls distance joint options. */
	enum class BS_SCRIPT_EXPORT(m:Physics) DistanceJointFlag
	{
		MinDistance = 0x1, /**< Enables minimum distance limit. */
		MaxDistance = 0x2, /**< Enables maximum distance limit. */
		Spring = 0x4 /**< Enables spring when maintaining limits. */
	};

	/** A joint that maintains an upper or lower (or both) bound on the distance between two bodies. */
	class BS_CORE_EXPORT DistanceJoint : public Joint
	{
	public:
		DistanceJoint(const DISTANCE_JOINT_DESC& desc) { }
		virtual ~DistanceJoint() = default;

		/** Returns the current distance between the two joint bodies. */
		virtual float getDistance() const = 0;

		/** @copydoc setMinDistance() */
		virtual float getMinDistance() const = 0;

		/**
		 * Determines the minimum distance the bodies are allowed to be at, they will get no closer. You must enable min
		 * distance flag in order for this limit to be applied.
		 */
		virtual void setMinDistance(float value) = 0;

		/** @copydoc setMaxDistance() */
		virtual float getMaxDistance() const = 0;

		/**
		 * Determines the maximum distance the bodies are allowed to be at, they will get no further. You must enable max
		 * distance flag in order for this limit to be applied.
		 */
		virtual void setMaxDistance(float value) = 0;

		/** @copydoc setTolerance() */
		virtual float getTolerance() const = 0;

		/**
		 * Determines the error tolerance of the joint at which the joint becomes active. This value slightly extends the
		 * lower and upper limit. 
		 */
		virtual void setTolerance(float value) = 0;

		/** @copydoc setSpring() */
		virtual Spring getSpring() const = 0;

		/**
		 * Determines a spring that controls how the joint responds when a limit is reached. You must enable the spring 
		 * flag on the joint in order for this to be recognized. 
		 * 
		 * @see	Spring
		 */
		virtual void setSpring(const Spring& value) = 0;

		/** Enables or disables a flag that controls joint behaviour. */
		virtual void setFlag(DistanceJointFlag flag, bool enabled) = 0;

		/** Checks whether a certain joint flag is enabled. */
		virtual bool hasFlag(DistanceJointFlag flag) const = 0;

		/** 
		 * Creates a new distance joint. 
		 * 
		 * @param[in]	scene		Scene to which to add the joint.
		 * @param[in]	desc		Settings describing the joint.
		 */
		static SPtr<DistanceJoint> create(PhysicsScene& scene, const DISTANCE_JOINT_DESC& desc);
	};

	/** Structure used for initializing a new DistanceJoint. */
	struct DISTANCE_JOINT_DESC : JOINT_DESC
	{
		float minDistance = 0.0f;
		float maxDistance = 0.0f;
		float tolerance = 0.25f;
		Spring spring;
		DistanceJointFlag flag = (DistanceJointFlag)0;
	};

	/** @} */
}