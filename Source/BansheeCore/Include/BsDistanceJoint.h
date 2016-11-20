//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsJoint.h"

namespace bs
{
	/** @addtogroup Physics
	 *  @{
	 */

	struct DISTANCE_JOINT_DESC;

	/** A joint that maintains an upper or lower (or both) bound on the distance between two bodies. */
	class BS_CORE_EXPORT DistanceJoint : public Joint
	{
	public:
		/** Controls distance joint options. */
		enum class Flag
		{
			MinDistance = 0x1, /** Enables minimum distance limit. */
			MaxDistance = 0x2, /** Enables maximum distance limit. */
			Spring = 0x4 /** Enables spring when maintaining limits. */
		};

	public:
		DistanceJoint(const DISTANCE_JOINT_DESC& desc) { }
		virtual ~DistanceJoint() { }

		/** Returns the current distance between the two joint bodies. */
		virtual float getDistance() const = 0;

		/**
		 * Returns the minimum distance the bodies are allowed to be at, they will get no closer. You must enable min
		 * distance flag in order for this limit to be applied.
		 */
		virtual float getMinDistance() const = 0;

		/**
		 * Sets the minimum distance the bodies are allowed to be at, they will get no closer. You must enable min
		 * distance flag in order for this limit to be applied.
		 */
		virtual void setMinDistance(float value) = 0;

		/**
		 * Returns the maximum distance the bodies are allowed to be at, they will get no further. You must enable max
		 * distance flag in order for this limit to be applied.
		 */
		virtual float getMaxDistance() const = 0;

		/**
		 * Sets the maximum distance the bodies are allowed to be at, they will get no further. You must enable max
		 * distance flag in order for this limit to be applied.
		 */
		virtual void setMaxDistance(float value) = 0;

		/**
		 * Returns the error tolerance of the joint at which the joint becomes active. This value slightly extends the
		 * lower and upper limit. 
		 */
		virtual float getTolerance() const = 0;

		/**
		 * Sets the error tolerance of the joint at which the joint becomes active. This value slightly extends the
		 * lower and upper limit. 
		 */
		virtual void setTolerance(float value) = 0;

		/**
		 * Returns a spring that controls how the joint responds when a limit is reached. You must enable the spring flag
		 * on the joint in order for this to be recognized. 
		 * 
		 * @see	Spring
		 */
		virtual Spring getSpring() const = 0;

		/**
		 * Sets a spring that controls how the joint responds when a limit is reached. You must enable the spring flag on
		 * the joint in order for this to be recognized. 
		 * 
		 * @see	Spring
		 */
		virtual void setSpring(const Spring& value) = 0;

		/** Enables or disables a flag that controls joint behaviour. */
		virtual void setFlag(Flag flag, bool enabled) = 0;

		/** Checks whether a certain joint flag is enabled. */
		virtual bool hasFlag(Flag flag) const = 0;

		/** Creates a new distance joint. */
		static SPtr<DistanceJoint> create(const DISTANCE_JOINT_DESC& desc);
	};

	/** Structure used for initializing a new DistanceJoint. */
	struct DISTANCE_JOINT_DESC : JOINT_DESC
	{
		float minDistance = 0.0f;
		float maxDistance = 0.0f;
		float tolerance = 0.25f;
		Spring spring;
		DistanceJoint::Flag flag = (DistanceJoint::Flag)0;
	};

	/** @} */
}