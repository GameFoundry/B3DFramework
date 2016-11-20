//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsFMODPrerequisites.h"
#include "BsAudioListener.h"

namespace bs
{
	/** @addtogroup FMOD
	 *  @{
	 */
	
	/** FMOD implementation of an AudioListener. */
	class FMODAudioListener : public AudioListener
	{
	public:
		FMODAudioListener();
		virtual ~FMODAudioListener();

		/** @copydoc AudioListener::setPosition */
		void setPosition(const Vector3& position) override;

		/** @copydoc AudioListener::setDirection */
		void setDirection(const Vector3& direction) override;

		/** @copydoc AudioListener::setUp */
		void setUp(const Vector3& up) override;

		/** @copydoc AudioListener::setVelocity */
		void setVelocity(const Vector3& velocity) override;

	private:
		friend class FMODAudio;

		/** Called by the FMODAudio system when the listener list changes. */
		void rebuild(INT32 id);

		INT32 mId;
	};

	/** @} */
}