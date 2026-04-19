//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DOAPrerequisites.h"
#include "Components/B3DAudioListener.h"
#include "Math/B3DTransform.h"

namespace b3d
{
	/** @addtogroup OpenAudio
	 *  @{
	 */

	/** OpenAL implementation of an AudioListener. */
	class OAAudioListener : public IAudioListenerImplementation
	{
	public:
		OAAudioListener();
		~OAAudioListener() override;

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

		Transform mTransform;
		Vector3 mVelocity = Vector3::kZero;
	};

	/** @} */
} // namespace b3d
