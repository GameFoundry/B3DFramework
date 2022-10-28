//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsOAPrerequisites.h"
#include "Audio/BsAudioListener.h"

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

		void SetTransform(const Transform& transform) override;
		void SetVelocity(const Vector3& velocity) override;

	private:
		friend class OAAudio;

		/** Re-applies stored properties to the listener. */
		void Rebuild();

		/** Returns forward and up direction as a single vector. */
		inline std::array<float, 6> GetOrientation() const;

		/** Updates internal position of the listener. */
		inline void UpdatePosition();

		/** Updates internal forward and up directions of the listener. */
		inline void UpdateOrientation(const std::array<float, 6>& orientation);

		/** Updates internal velocity of the listener. */
		inline void UpdateVelocity();

		/** Updates internal volume of the listener. */
		inline void UpdateVolume(float volume);
	};

	/** @} */
} // namespace bs
