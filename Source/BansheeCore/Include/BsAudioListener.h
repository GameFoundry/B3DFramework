//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsVector3.h"

namespace BansheeEngine
{
	/** @addtogroup Audio
	 *  @{
	 */

	/** 
	 * Represents a listener that hears audio sources. For spatial audio the volume and pitch of played audio is determined
	 * by the distance, orientation and velocity differences between the source and the listener.
	 */
	class BS_CORE_EXPORT AudioListener
	{
	public:
		virtual ~AudioListener() {}

		/** Sets the position of the listener in world space. */
		virtual void setPosition(const Vector3& position);

		/** Retrieves the position of the listener in world space. */
		Vector3 getPosition() const { return mPosition; }

		/** Sets the forward direction of the listener in world space. */
		virtual void setDirection(const Vector3& direction);

		/** Retrieves the forward direction of the listener in world space. */
		Vector3 getDirection() const { return mDirection; }

		/** Sets the up direction of the listener in world space. */
		virtual void setUp(const Vector3& up);

		/** Retrieves the up direction of the listener in world space. */
		Vector3 getUp() const { return mUp; }

		/** Sets the velocity of the listener. */
		virtual void setVelocity(const Vector3& velocity);

		/** Retrieves the velocity of the listener. */
		Vector3 getVelocity() const { return mVelocity; }

		/** Creates a new audio listener. */
		static SPtr<AudioListener> create();

	protected:
		AudioListener();

		Vector3 mPosition;
		Vector3 mDirection;
		Vector3 mUp;
		Vector3 mVelocity;
	};

	/** @} */
}