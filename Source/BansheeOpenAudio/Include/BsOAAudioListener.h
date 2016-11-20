//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsOAPrerequisites.h"
#include "BsAudioListener.h"

namespace bs
{
	/** @addtogroup OpenAudio
	 *  @{
	 */
	
	/** OpenAL implementation of an AudioListener. */
	class OAAudioListener : public AudioListener
	{
	public:
		OAAudioListener();
		virtual ~OAAudioListener();

		/** @copydoc AudioListener::setPosition */
		void setPosition(const Vector3& position) override;

		/** @copydoc AudioListener::setDirection */
		void setDirection(const Vector3& direction) override;

		/** @copydoc AudioListener::setUp */
		void setUp(const Vector3& up) override;

		/** @copydoc AudioListener::setVelocity */
		void setVelocity(const Vector3& velocity) override;

	private:
		friend class OAAudio;

		/** Re-applies stored properties to the listener. */
		void rebuild();

		/** Returns forward and up direction as a single vector. */
		inline std::array<float, 6> getOrientation() const;

		/** Updates internal position of the listener. */
		inline void updatePosition();

		/** Updates internal forward and up directions of the listener. */
		inline void updateOrientation(const std::array<float, 6>& orientation);

		/** Updates internal velocity of the listener. */
		inline void updateVelocity();

		/** Updates internal volume of the listener. */
		inline void updateVolume(float volume);
	};

	/** @} */
}