//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Scene/BsSceneActor.h"
#include "Math/BsVector3.h"

namespace bs
{
	/** @addtogroup Audio
	 *  @{
	 */

	/** 
	 * Represents a listener that hears audio sources. For spatial audio the volume and pitch of played audio is determined
	 * by the distance, orientation and velocity differences between the source and the listener.
	 */
	class BS_CORE_EXPORT AudioListener : public IReflectable, public SceneActor
	{
	public:
		virtual ~AudioListener() {}

		/** Sets the velocity of the listener. */
		virtual void setVelocity(const Vector3& velocity);

		/** Retrieves the velocity of the listener. */
		Vector3 getVelocity() const { return mVelocity; }

		/** Creates a new audio listener. */
		static SPtr<AudioListener> create();

	protected:
		AudioListener();

		Vector3 mVelocity;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class AudioListenerRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}